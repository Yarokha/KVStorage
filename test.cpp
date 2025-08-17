#include "kv_storage.h"
#include <gtest/gtest.h>
#include <chrono>

using namespace std::chrono_literals;

// абстрактные часы инициализированные реальным начальным временем
struct MyClock {
    using time_point = std::chrono::system_clock::time_point;
    static time_point cur_time;

    static time_point now() {
        return cur_time;
    }
    static void forward(std::chrono::seconds seconds) {
        cur_time += seconds;
    }
    static time_point max() {
        return time_point::max();
    }
};

MyClock::time_point MyClock::cur_time{ std::chrono::system_clock::now() };


// получение данных созданных конструктором в тот же момент времени 
TEST(KVStorageTest, InitializationTest) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> test_data{ {"a", "val1",10}, {"b", "val2",20}, {"d", "val3", 100}, {"e", "val4",0} };
    KVStorage<MyClock> data(test_data);

    EXPECT_EQ("val1", data.get("a"));
    EXPECT_EQ("val2", data.get("b"));
    EXPECT_EQ("val3", data.get("d"));
    EXPECT_EQ("val4", data.get("e"));
}


// получение данных созданных set в тот же момент времени 
TEST(KVStorageTest, setTest) {
    KVStorage<MyClock> data({});

    data.set("a", "val1", 10);
    data.set("b", "val2", 20);
    data.set("d", "val3", 100);
    data.set("e", "val4", 0);

    EXPECT_EQ("val1", data.get("a"));
    EXPECT_EQ("val2", data.get("b"));
    EXPECT_EQ("val3", data.get("d"));
    EXPECT_EQ("val4", data.get("e"));
}


// получение данных, двойное(существующего ключа и уже удаленного) удаление по ключу и доп проверка, что удалено по ключу 
TEST(KVStorageTest, removeTest) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> test_data{ {"a", "val1",10}, {"b", "val2",20}, {"d", "val3", 100}, {"e", "val4",0} };
    KVStorage<MyClock> data(test_data);
    EXPECT_EQ("val1", data.get("a"));
    EXPECT_TRUE(data.remove("a"));
    EXPECT_FALSE(data.remove("a"));
    EXPECT_EQ(std::nullopt, data.get("a"));
}


// получение данных после протухания некоторых данных 
TEST(KVStorageTest, getTest) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> test_data{ {"a", "val1",10}, {"b", "val2",20}, {"d", "val3", 100}, {"e", "val4",0} };
    KVStorage<MyClock> data(test_data);
    EXPECT_EQ("val1", data.get("a"));
    MyClock::forward(50s); // сдвивигаем время на 50 секунд
    EXPECT_EQ(std::nullopt, data.get("a"));
}


// получение данных в лексикографическом порядке
TEST(KVStorageTest, getManySortedTest) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> test_data{ {"a", "val1",10}, {"b", "val2",20}, {"d", "val3", 100}, {"e", "val4",0} };
    KVStorage<MyClock> data(test_data);

    auto check_before_exp{ data.getManySorted("b", 3) };
    EXPECT_EQ(3, check_before_exp.size());

    MyClock::forward(2min); // сдвивигаем время на 2 минуты
    auto check_after_exp{ data.getManySorted("b", 3) };
    EXPECT_EQ(1, check_after_exp.size());
}


// получение протухших данных и удаление из базы 
TEST(KVStorageTest, removeOneExpiredEntryTest) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> test_data{ {"a", "val1",10}, {"b", "val2",20}, {"d", "val3", 100}, {"e", "val4",0} };
    KVStorage<MyClock> data(test_data);
    MyClock::forward(15s); // сдвивигаем время на 15 секунд 1 протухшие данные
    
    auto check_1{ data.removeOneExpiredEntry() };
    ASSERT_TRUE(check_1.has_value());
    auto& [key, value] = check_1.value();
    EXPECT_EQ("a", key);
    EXPECT_EQ("val1", value);

    auto check_2{ data.removeOneExpiredEntry() };
    EXPECT_EQ(std::nullopt, check_2);
}


// перепись данных просле протухания
TEST(KVStorageTest, setToRewriteTest) {
    std::vector<std::tuple<std::string, std::string, uint32_t>> test_data{ {"a", "val1",10}, {"b", "val2",20}, {"d", "val3", 100}, {"e", "val4",0} };
    KVStorage<MyClock> data(test_data);
    MyClock::forward(15s); // сдвивигаем время на 15 секунд 1 протухшие данные
    EXPECT_EQ(std::nullopt, data.get("a")); // данные протухли

    data.set("a", "val1", 0); // переписали
    MyClock::forward(100h); // сдвивигаем время на 100 часов
    EXPECT_EQ("val1", data.get("a")); // данные доступны
}