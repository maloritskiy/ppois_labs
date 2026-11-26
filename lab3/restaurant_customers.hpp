#pragma once

#include "restaurant_core.hpp"
#include "restaurant_exceptions.hpp"

#include <algorithm>
#include <string>
#include <vector>
#include <utility>

namespace restaurant {

class CustomerProfile {
private:
    std::string phoneNumber_;
    std::string email_;
    std::string birthDate_;
    std::vector<std::string> preferences_;
    int visitCount_ = 0;

public:
    CustomerProfile() = default;
    CustomerProfile(std::string phone, std::string email, std::string birthDate)
        : phoneNumber_(std::move(phone)), email_(std::move(email)), birthDate_(std::move(birthDate)) {}

    const std::string& phoneNumber() const { return phoneNumber_; }
    const std::string& email() const { return email_; }
    const std::string& birthDate() const { return birthDate_; }
    void addPreference(const std::string& value) {
        if (!value.empty() && std::find(preferences_.begin(), preferences_.end(), value) == preferences_.end()) preferences_.push_back(value);
    }
    void removePreference(const std::string& value) {
        preferences_.erase(std::remove(preferences_.begin(), preferences_.end(), value), preferences_.end());
    }
    bool hasPreference(const std::string& value) const {
        return std::find(preferences_.begin(), preferences_.end(), value) != preferences_.end();
    }
    void recordVisit() { ++visitCount_; }
    int visitCount() const { return visitCount_; }
    bool contactIsValid() const { return !phoneNumber_.empty() || email_.find('@') != std::string::npos; }
};

class LoyaltyCard {
private:
    std::string cardNumber_;
    int points_ = 0;
    std::string tier_ = "Standard";
    bool active_ = true;
    int totalPointsEarned_ = 0;

public:
    LoyaltyCard() = default;
    explicit LoyaltyCard(std::string number) : cardNumber_(std::move(number)) {
        if (cardNumber_.empty()) throw RestaurantException("Номер карты лояльности не может быть пустым");
    }

    const std::string& number() const { return cardNumber_; }
    int points() const { return points_; }
    const std::string& tier() const { return tier_; }
    void addPoints(int value) {
        if (!active_ || value < 0) throw RestaurantException("Нельзя начислить баллы на эту карту");
        points_ += value;
        totalPointsEarned_ += value;
        updateTier();
    }
    bool redeemPoints(int value) {
        if (!active_ || value <= 0 || value > points_) return false;
        points_ -= value;
        updateTier();
        return true;
    }
    void deactivate() { active_ = false; }
    void activate() { active_ = true; }
    bool isActive() const { return active_; }
    void updateTier() {
        if (totalPointsEarned_ >= 1000) tier_ = "Gold";
        else if (totalPointsEarned_ >= 300) tier_ = "Silver";
        else tier_ = "Standard";
    }
};

class Customer {
private:
    int customerId_ = 0;
    std::string fullName_;
    CustomerProfile profile_;
    LoyaltyCard loyaltyCard_;
    std::string registrationDate_;
    bool active_ = true;

public:
    Customer() = default;
    Customer(int id, std::string name, CustomerProfile profile, LoyaltyCard card, std::string registrationDate)
        : customerId_(id), fullName_(std::move(name)), profile_(std::move(profile)),
          loyaltyCard_(std::move(card)), registrationDate_(std::move(registrationDate)) {
        if (id <= 0 || fullName_.empty()) throw RestaurantException("Некорректные данные клиента");
    }

    int id() const { return customerId_; }
    const std::string& name() const { return fullName_; }
    const std::string& registrationDate() const { return registrationDate_; }
    CustomerProfile& profile() { return profile_; }
    LoyaltyCard& loyaltyCard() { return loyaltyCard_; }
    void deactivate() { active_ = false; }
    void activate() { active_ = true; }
    bool isActive() const { return active_; }
    bool canMakeReservation() const { return active_ && profile_.contactIsValid(); }
    void recordVisit() { profile_.recordVisit(); }
};

enum class ReservationStatus { Pending, Confirmed, Seated, Cancelled, Completed };

class Reservation {
private:
    int reservationId_ = 0;
    Customer customer_;
    int tableId_ = 0;
    int guestCount_ = 0;
    std::string date_;
    int startMinute_ = 0;
    int durationMinutes_ = 0;
    ReservationStatus status_ = ReservationStatus::Pending;

public:
    Reservation() = default;
    Reservation(int id, const Customer& customer, int tableId, int guests,
                std::string date, int startMinute, int durationMinutes)
        : reservationId_(id), customer_(customer), tableId_(tableId), guestCount_(guests),
          date_(std::move(date)), startMinute_(startMinute), durationMinutes_(durationMinutes) {
        if (id <= 0 || !customer.canMakeReservation() || tableId <= 0 || guests <= 0 || date_.empty() ||
            startMinute < 0 || startMinute >= 1440 || durationMinutes <= 0 || startMinute + durationMinutes > 1440)
            throw InvalidReservationException("Некорректные данные бронирования");
    }

    int id() const { return reservationId_; }
    int tableId() const { return tableId_; }
    const std::string& date() const { return date_; }
    int startMinute() const { return startMinute_; }
    int durationMinutes() const { return durationMinutes_; }
    int guestCount() const { return guestCount_; }
    ReservationStatus status() const { return status_; }
    const Customer& customer() const { return customer_; }
    bool overlaps(const Reservation& other) const {
        if (date_ != other.date_ || tableId_ != other.tableId_ ||
            status_ == ReservationStatus::Cancelled || status_ == ReservationStatus::Completed ||
            other.status_ == ReservationStatus::Cancelled || other.status_ == ReservationStatus::Completed)
            return false;
        return startMinute_ < other.startMinute_ + other.durationMinutes_ &&
               other.startMinute_ < startMinute_ + durationMinutes_;
    }
    void confirm(RestaurantTable& table) {
        if (status_ != ReservationStatus::Pending || table.id() != tableId_ ||
            table.capacity() < guestCount_ || table.status() != TableStatus::Available)
            throw InvalidReservationException("Нельзя подтвердить бронирование");
        table.reserve();
        status_ = ReservationStatus::Confirmed;
    }
    void markSeated(RestaurantTable& table) {
        if (status_ != ReservationStatus::Confirmed || table.id() != tableId_ ||
            table.status() != TableStatus::Reserved)
            throw InvalidReservationException("Нельзя зарегистрировать посадку по этому бронированию");
        table.occupy();
        status_ = ReservationStatus::Seated;
    }
    void cancel() {
        if (status_ == ReservationStatus::Confirmed)
            throw InvalidReservationException("Для отмены подтверждённого бронирования укажите столик");
        if (status_ == ReservationStatus::Seated || status_ == ReservationStatus::Completed)
            throw InvalidReservationException("Начатое или завершённое посещение нельзя отменить");
        status_ = ReservationStatus::Cancelled;
    }
    void cancel(RestaurantTable& table) {
        if (status_ != ReservationStatus::Confirmed || table.id() != tableId_ ||
            table.status() != TableStatus::Reserved)
            throw InvalidReservationException("Нельзя отменить это бронирование для указанного столика");
        table.release();
        status_ = ReservationStatus::Cancelled;
    }
    void complete(RestaurantTable& table) {
        if (status_ != ReservationStatus::Seated || table.id() != tableId_ ||
            table.status() != TableStatus::Occupied)
            throw InvalidReservationException("Бронирование нельзя завершить для этого столика");
        table.markNeedsCleaning();
        status_ = ReservationStatus::Completed;
    }
    bool isForCustomer(int customerId) const { return customer_.id() == customerId; }
};

class WaitlistEntry {
private:
    int entryId_ = 0;
    Customer customer_;
    int guestCount_ = 0;
    std::string preferredDate_;
    int preferredStartMinute_ = 0;
    std::string note_;
    bool active_ = true;

public:
    WaitlistEntry() = default;
    WaitlistEntry(int id, const Customer& customer, int guests, std::string date, int minute, std::string note = {})
        : entryId_(id), customer_(customer), guestCount_(guests), preferredDate_(std::move(date)),
          preferredStartMinute_(minute), note_(std::move(note)) {
        if (id <= 0 || !customer.canMakeReservation() || guests <= 0 || preferredDate_.empty() || minute < 0 || minute >= 1440)
            throw InvalidReservationException("Некорректная запись листа ожидания");
    }

    int id() const { return entryId_; }
    int guestCount() const { return guestCount_; }
    const std::string& preferredDate() const { return preferredDate_; }
    int preferredStartMinute() const { return preferredStartMinute_; }
    const std::string& note() const { return note_; }
    bool isActive() const { return active_; }
    bool matchesTable(const RestaurantTable& table) const { return active_ && table.canAccommodate(guestCount_); }
    void notifyAvailability(const RestaurantTable& table) {
        if (!matchesTable(table)) throw TableUnavailableException("Столик не подходит записи ожидания");
        note_ = "Доступен столик " + std::to_string(table.number());
    }
    void removeFromWaitlist() { active_ = false; }
    bool belongsToCustomer(int customerId) const { return customer_.id() == customerId; }
};

} // namespace restaurant