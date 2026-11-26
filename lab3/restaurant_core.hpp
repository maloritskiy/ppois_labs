#pragma once

#include "restaurant_menu.hpp"
#include "restaurant_exceptions.hpp"

#include <algorithm>
#include <string>
#include <vector>
#include <utility>

namespace restaurant {

class RestaurantAddress {
private:
    std::string street_;
    std::string buildingNumber_;
    std::string city_;
    std::string postalCode_;
    std::string country_;

public:
    RestaurantAddress() = default;
    RestaurantAddress(std::string street, std::string building, std::string city,
                      std::string postalCode, std::string country)
        : street_(std::move(street)), buildingNumber_(std::move(building)),
          city_(std::move(city)), postalCode_(std::move(postalCode)), country_(std::move(country)) {}

    const std::string& city() const { return city_; }
    bool isValid() const {
        return !street_.empty() && !buildingNumber_.empty() && !city_.empty() &&
               !postalCode_.empty() && !country_.empty();
    }
    std::string formatFullAddress() const {
        return street_ + ", " + buildingNumber_ + ", " + city_ + ", " + postalCode_ + ", " + country_;
    }
    bool isLocatedIn(const std::string& cityName) const { return city_ == cityName; }
};

class OpeningHours {
private:
    std::string dayOfWeek_;
    int openingMinute_ = 0;
    int closingMinute_ = 0;
    bool closed_ = true;
    std::string specialNote_;

public:
    OpeningHours() = default;
    OpeningHours(std::string day, int opening, int closing, bool closed = false, std::string note = {})
        : dayOfWeek_(std::move(day)), openingMinute_(opening), closingMinute_(closing),
          closed_(closed), specialNote_(std::move(note)) {
        if (!isValid()) throw RestaurantException("Некорректный график работы");
    }

    const std::string& day() const { return dayOfWeek_; }
    const std::string& specialNote() const { return specialNote_; }
    void setSpecialNote(const std::string& note) { specialNote_ = note; }
    bool isClosed() const { return closed_; }
    bool isValid() const {
        return !dayOfWeek_.empty() && openingMinute_ >= 0 && openingMinute_ < 1440 &&
               closingMinute_ >= 0 && closingMinute_ <= 1440 &&
               (closed_ || closingMinute_ > openingMinute_);
    }
    bool isOpenAt(int hour, int minute) const {
        if (closed_ || hour < 0 || hour > 23 || minute < 0 || minute > 59) return false;
        const int value = hour * 60 + minute;
        return value >= openingMinute_ && value < closingMinute_;
    }
    int getDurationMinutes() const { return closed_ ? 0 : closingMinute_ - openingMinute_; }
    void setWorkingPeriod(int opening, int closing) {
        if (opening < 0 || opening >= 1440 || closing <= opening || closing > 1440)
            throw RestaurantException("Некорректный рабочий интервал");
        openingMinute_ = opening;
        closingMinute_ = closing;
        closed_ = false;
    }
    void closeForDay() { closed_ = true; }
};

class TableZone {
private:
    int zoneId_ = 0;
    std::string name_;
    int floorNumber_ = 1;
    bool outdoor_ = false;
    bool quiet_ = false;
    std::vector<std::string> features_;

public:
    TableZone() = default;
    TableZone(int id, std::string name, int floor, bool outdoor, bool quiet)
        : zoneId_(id), name_(std::move(name)), floorNumber_(floor), outdoor_(outdoor), quiet_(quiet) {
        if (id <= 0 || name_.empty() || floor <= 0) throw RestaurantException("Некорректная зона");
    }

    int id() const { return zoneId_; }
    const std::string& name() const { return name_; }
    int floor() const { return floorNumber_; }
    bool matchesPreferences(bool quiet, bool outdoor) const {
        return (!quiet || quiet_) && (!outdoor || outdoor_);
    }
    bool hasFeature(const std::string& feature) const {
        return std::find(features_.begin(), features_.end(), feature) != features_.end();
    }
    void addFeature(const std::string& feature) {
        if (!feature.empty() && !hasFeature(feature)) features_.push_back(feature);
    }
    void removeFeature(const std::string& feature) {
        features_.erase(std::remove(features_.begin(), features_.end(), feature), features_.end());
    }
};

enum class TableStatus { Available, Reserved, Occupied, NeedsCleaning, OutOfService };

class RestaurantTable {
private:
    int tableId_ = 0;
    int tableNumber_ = 0;
    int capacity_ = 0;
    TableStatus status_ = TableStatus::Available;
    bool nearWindow_ = false;
    int zoneId_ = 0;

public:
    RestaurantTable() = default;
    RestaurantTable(int id, int number, int capacity, bool nearWindow, int zoneId)
        : tableId_(id), tableNumber_(number), capacity_(capacity), nearWindow_(nearWindow), zoneId_(zoneId) {
        if (id <= 0 || number <= 0 || capacity <= 0 || zoneId <= 0)
            throw RestaurantException("Некорректные данные столика");
    }

    int id() const { return tableId_; }
    int number() const { return tableNumber_; }
    int capacity() const { return capacity_; }
    int zoneId() const { return zoneId_; }
    bool isNearWindow() const { return nearWindow_; }
    TableStatus status() const { return status_; }
    bool canAccommodate(int guests) const { return guests > 0 && guests <= capacity_ && status_ == TableStatus::Available; }
    void reserve() {
        if (status_ != TableStatus::Available) throw TableUnavailableException("Столик недоступен для бронирования");
        status_ = TableStatus::Reserved;
    }
    void occupy() {
        if (status_ != TableStatus::Available && status_ != TableStatus::Reserved)
            throw TableUnavailableException("Столик нельзя занять в текущем состоянии");
        status_ = TableStatus::Occupied;
    }
    void release() {
        if (status_ != TableStatus::Reserved)
            throw TableUnavailableException("Освободить таким способом можно только забронированный столик");
        status_ = TableStatus::Available;
    }
    void markNeedsCleaning() {
        if (status_ != TableStatus::Occupied)
            throw TableUnavailableException("Убирать можно только занятый столик");
        status_ = TableStatus::NeedsCleaning;
    }
    void markCleaned() {
        if (status_ != TableStatus::NeedsCleaning)
            throw TableUnavailableException("Столик не ожидает уборки");
        status_ = TableStatus::Available;
    }
    void takeOutOfService() { status_ = TableStatus::OutOfService; }
    void restoreService() {
        if (status_ != TableStatus::OutOfService) throw RestaurantException("Столик не выведен из эксплуатации");
        status_ = TableStatus::NeedsCleaning;
    }
    bool matchesPreferences(bool nearWindow) const { return !nearWindow || nearWindow_; }
    void assignZone(int zoneId) { if (zoneId <= 0) throw RestaurantException("Некорректный идентификатор зоны"); zoneId_ = zoneId; }
};

class DiningHall {
private:
    int hallId_ = 0;
    std::string name_;
    int floorNumber_ = 1;
    int capacity_ = 0;
    bool open_ = true;
    std::vector<RestaurantTable> tables_;
    std::vector<TableZone> zones_;

public:
    DiningHall() = default;
    DiningHall(int id, std::string name, int floor, int capacity)
        : hallId_(id), name_(std::move(name)), floorNumber_(floor), capacity_(capacity) {
        if (id <= 0 || name_.empty() || floor <= 0 || capacity <= 0)
            throw RestaurantException("Некорректные данные зала");
    }

    int id() const { return hallId_; }
    const std::string& name() const { return name_; }
    int floor() const { return floorNumber_; }
    int capacity() const { return capacity_; }
    const std::vector<RestaurantTable>& tables() const { return tables_; }
    void addTable(const RestaurantTable& table) {
        if (findTable(table.number()) != nullptr) throw RestaurantException("Столик с таким номером уже существует");
        int currentCapacity = 0;
        for (const auto& existing : tables_) currentCapacity += existing.capacity();
        if (currentCapacity + table.capacity() > capacity_) throw RestaurantException("Превышена вместимость зала");
        tables_.push_back(table);
    }
    void removeTable(int tableNumber) {
        auto it = std::remove_if(tables_.begin(), tables_.end(), [tableNumber](const RestaurantTable& table) { return table.number() == tableNumber; });
        if (it == tables_.end()) throw TableNotFoundException("Столик не найден");
        tables_.erase(it, tables_.end());
    }
    void addZone(const TableZone& zone) {
        auto it = std::find_if(zones_.begin(), zones_.end(), [&zone](const TableZone& item) { return item.id() == zone.id(); });
        if (it != zones_.end()) throw RestaurantException("Зона уже существует");
        zones_.push_back(zone);
    }
    void assignTableToZone(int tableNumber, int zoneId) {
        auto table = std::find_if(tables_.begin(), tables_.end(), [tableNumber](const RestaurantTable& item) { return item.number() == tableNumber; });
        auto zone = std::find_if(zones_.begin(), zones_.end(), [zoneId](const TableZone& item) { return item.id() == zoneId; });
        if (table == tables_.end()) throw TableNotFoundException("Столик не найден");
        if (zone == zones_.end()) throw RestaurantException("Зона не найдена");
        table->assignZone(zoneId);
    }
    RestaurantTable* findTable(int tableNumber) {
        auto it = std::find_if(tables_.begin(), tables_.end(), [tableNumber](const RestaurantTable& item) { return item.number() == tableNumber; });
        return it == tables_.end() ? nullptr : &*it;
    }
    RestaurantTable* findTableById(int tableId) {
        auto it = std::find_if(tables_.begin(), tables_.end(), [tableId](const RestaurantTable& item) { return item.id() == tableId; });
        return it == tables_.end() ? nullptr : &*it;
    }
    const RestaurantTable* findTable(int tableNumber) const {
        auto it = std::find_if(tables_.begin(), tables_.end(), [tableNumber](const RestaurantTable& item) { return item.number() == tableNumber; });
        return it == tables_.end() ? nullptr : &*it;
    }
    std::vector<int> findAvailableTables(int guests) const {
        std::vector<int> result;
        for (const auto& table : tables_) if (table.canAccommodate(guests)) result.push_back(table.number());
        return result;
    }
    int calculateFreeSeats() const {
        int result = 0;
        for (const auto& table : tables_) if (table.status() == TableStatus::Available) result += table.capacity();
        return result;
    }
    void open() { open_ = true; }
    void close() { open_ = false; }
    bool isOpen() const { return open_; }
};

class Restaurant {
private:
    int restaurantId_ = 0;
    std::string name_;
    std::string phoneNumber_;
    RestaurantAddress address_;
    std::vector<OpeningHours> openingHours_;
    std::vector<DiningHall> halls_;
    Menu activeMenu_;

public:
    Restaurant() = default;
    Restaurant(int id, std::string name, std::string phone, RestaurantAddress address)
        : restaurantId_(id), name_(std::move(name)), phoneNumber_(std::move(phone)), address_(std::move(address)) {
        if (id <= 0 || name_.empty() || !address_.isValid()) throw RestaurantException("Некорректные данные ресторана");
    }

    int id() const { return restaurantId_; }
    const std::string& name() const { return name_; }
    const std::string& phoneNumber() const { return phoneNumber_; }
    const RestaurantAddress& address() const { return address_; }
    const std::vector<DiningHall>& halls() const { return halls_; }
    const Menu& activeMenu() const { return activeMenu_; }
    void addHall(const DiningHall& hall) {
        auto it = std::find_if(halls_.begin(), halls_.end(), [&hall](const DiningHall& item) { return item.id() == hall.id(); });
        if (it != halls_.end()) throw RestaurantException("Зал уже существует");
        halls_.push_back(hall);
    }
    void removeHall(int hallId) {
        auto it = std::remove_if(halls_.begin(), halls_.end(), [hallId](const DiningHall& item) { return item.id() == hallId; });
        if (it == halls_.end()) throw RestaurantException("Зал не найден");
        halls_.erase(it, halls_.end());
    }
    RestaurantTable* findTable(int tableNumber) {
        for (auto& hall : halls_) if (RestaurantTable* table = hall.findTable(tableNumber)) return table;
        return nullptr;
    }
    RestaurantTable* findTableById(int tableId) {
        for (auto& hall : halls_) if (RestaurantTable* table = hall.findTableById(tableId)) return table;
        return nullptr;
    }
    bool isOpenAt(const std::string& day, int hour, int minute) const {
        auto it = std::find_if(openingHours_.begin(), openingHours_.end(), [&day](const OpeningHours& item) { return item.day() == day; });
        if (it == openingHours_.end()) return false;
        return it->isOpenAt(hour, minute);
    }
    void updateOpeningHours(const OpeningHours& hours) {
        auto it = std::find_if(openingHours_.begin(), openingHours_.end(), [&hours](const OpeningHours& item) { return item.day() == hours.day(); });
        if (it == openingHours_.end()) openingHours_.push_back(hours);
        else *it = hours;
    }
    void setActiveMenu(const Menu& menu) {
        if (menu.status() != MenuStatus::Active) throw RestaurantException("Можно назначить только активное меню");
        activeMenu_ = menu;
    }
    int calculateTotalCapacity() const {
        int total = 0;
        for (const auto& hall : halls_) for (const auto& table : hall.tables()) total += table.capacity();
        return total;
    }
    std::vector<int> findAvailableTables(int guests) const {
        std::vector<int> result;
        for (const auto& hall : halls_) {
            const auto numbers = hall.findAvailableTables(guests);
            result.insert(result.end(), numbers.begin(), numbers.end());
        }
        return result;
    }
};

} // namespace restaurant