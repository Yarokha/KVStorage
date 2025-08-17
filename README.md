# KVStorage

    explicit KVStorage(std::span<std::tuple<std::string, std::string , uint32_t>> entries, Clock clock = Clock());
        O(n log n), так как n проходов n вставок в set за log n
   

    void set(std::string key, std::string value, uint32_t ttl);
        O(log n), так как добавляется в set ключ
        
    bool remove(std::string_view key);
        O(log n), так как удаляем из set ключ


    std::optional<std::string> get(std::string_view key) const;
        За O(1) так как храним в unordered_set


    std::vector<std::pair<std::string, std::string>> getManySorted(std::string_view key, uint32_t count) const;
        За O(log n) так как есть бинарный поиск  и O(k) количесво пройденных записей итого O(log (n) + k)

    std::optional<std::pair<std::string, std::string>> removeOneExpiredEntry();
        как у remove O(log n)