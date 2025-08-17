#include <set>
#include <map>
#include <span>
#include <tuple>
#include <chrono>
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <unordered_set>

template<typename Clock>
class KVStorage {
public:
    explicit KVStorage(std::span<std::tuple<std::string /*key*/, std::string /*value*/, uint32_t /*ttl*/>> entries, Clock clock = Clock()) : clock_(clock) {
        for (auto& [key, value, ttl] : entries) {
            set(key, value, ttl);
        }
    }

    ~KVStorage() {}

    void set(std::string key, std::string value, uint32_t ttl) {
        if (data_.contains(key)) {
            exp_buckets_.erase(data_[key].exp_time);
        }
        else
            key_set_.insert(key);

        if (ttl == 0) {
            data_[key] = { value,  Clock::time_point::max() };
        }
        else {
            data_[key] = { value, clock_.now() + std::chrono::seconds(ttl) };
            exp_buckets_[data_[key].exp_time].insert(key);
        }
    }

    bool remove(std::string_view key) {
        auto to_remove{ data_.find(std::string(key)) };
        if (to_remove != data_.end()) {
            exp_buckets_[to_remove->second.exp_time].erase(to_remove->first);
            if (exp_buckets_[to_remove->second.exp_time].empty())
                exp_buckets_.erase(to_remove->second.exp_time);
            key_set_.erase(to_remove->first);
            data_.erase(to_remove->first);
            return true;
        }
        return false;
    }

    std::optional<std::string> get(std::string_view key) const {
        auto iter{ data_.find(std::string(key)) };
        if (iter != data_.end() && iter->second.exp_time > clock_.now())
            return iter->second.value;
        return std::nullopt;
    }

    std::vector<std::pair<std::string, std::string>> getManySorted(std::string_view key, uint32_t count) const {
        std::vector < std::pair<std::string, std::string>> result;
        result.reserve(count);
        for (auto iter{ key_set_.lower_bound(std::string(key)) }; iter != key_set_.end() && count != 0; ++iter) {
            if (data_.find(*iter)->second.exp_time <= clock_.now())
                continue;
            result.push_back({ std::string(*iter), data_.find(*iter)->second.value });
            --count;
        }
        return result;
    }

    std::optional<std::pair<std::string, std::string>> removeOneExpiredEntry() {
        if (!exp_buckets_.empty()) {
            auto exp_data_ = exp_buckets_.begin();
            if (exp_data_->first <= clock_.now()) {
                std::pair<std::string, std::string> result(*exp_data_->second.begin(), data_[*exp_data_->second.begin()].value);
                remove(*exp_data_->second.begin());
                return result;
            }
        }
        return std::nullopt;
    }

private:
    Clock clock_;
    struct Data {
        std::string value;
        Clock::time_point exp_time;
    };

    std::unordered_map<std::string, Data> data_;
    std::map<typename Clock::time_point, std::unordered_set<std::string>> exp_buckets_;
    std::set<std::string> key_set_;
};
