#pragma once

#include "restaurant_orders.hpp"
#include "restaurant_staff.hpp"
#include "restaurant_exceptions.hpp"

#include <string>
#include <utility>

namespace restaurant {

enum class DeliveryStatus { Created, Assigned, PickedUp, Delivered, Cancelled };

class DeliveryAddress {
private:
    std::string street_;
    std::string building_;
    std::string city_;
    std::string apartment_;
    std::string instructions_;

public:
    DeliveryAddress() = default;
    DeliveryAddress(std::string street, std::string building, std::string city,
                    std::string apartment, std::string instructions)
        : street_(std::move(street)), building_(std::move(building)), city_(std::move(city)),
          apartment_(std::move(apartment)), instructions_(std::move(instructions)) {}
    const std::string& instructions() const { return instructions_; }
    bool isValid() const { return !street_.empty() && !building_.empty() && !city_.empty(); }
    std::string format() const {
        std::string result = street_ + ", " + building_ + ", " + city_;
        if (!apartment_.empty()) result += ", кв. " + apartment_;
        return result;
    }
    bool isInCity(const std::string& city) const { return city_ == city; }
    void updateInstructions(const std::string& instructions) { instructions_ = instructions; }
};

class Courier {
private:
    int courierId_ = 0;
    std::string name_;
    std::string phoneNumber_;
    bool available_ = true;
    int completedDeliveries_ = 0;
    double rating_ = 5.0;
    int ratingCount_ = 0;

public:
    Courier() = default;
    Courier(int id, std::string name, std::string phone)
        : courierId_(id), name_(std::move(name)), phoneNumber_(std::move(phone)) {
        if (id <= 0 || name_.empty() || phoneNumber_.empty()) throw RestaurantException("Некорректные данные курьера");
    }
    int id() const { return courierId_; }
    const std::string& name() const { return name_; }
    const std::string& phoneNumber() const { return phoneNumber_; }
    bool isAvailable() const { return available_; }
    bool assignDelivery() {
        if (!available_) return false;
        available_ = false;
        return true;
    }
    void completeDelivery() {
        ++completedDeliveries_;
        available_ = true;
    }
    void setAvailable(bool value) { available_ = value; }
    void addRating(double value) {
        if (value < 1.0 || value > 5.0) throw RestaurantException("Оценка должна быть от 1 до 5");
        rating_ = (rating_ * ratingCount_ + value) / (ratingCount_ + 1);
        ++ratingCount_;
    }
    int completedDeliveries() const { return completedDeliveries_; }
    double rating() const { return rating_; }
};

class Delivery {
private:
    int deliveryId_ = 0;
    int orderId_ = 0;
    int customerId_ = 0;
    DeliveryAddress address_;
    int courierId_ = 0;
    DeliveryStatus status_ = DeliveryStatus::Created;
    double fee_ = 0.0;
    std::string createdAt_;

public:
    Delivery() = default;
    Delivery(int id, const Order& order, const Customer& customer, DeliveryAddress address,
             double fee, std::string createdAt)
        : deliveryId_(id), orderId_(order.id()), customerId_(customer.id()), address_(std::move(address)),
          fee_(fee), createdAt_(std::move(createdAt)) {
        if (id <= 0 || order.items().empty() || !customer.isActive() || !address_.isValid() || fee < 0.0 || createdAt_.empty())
            throw DeliveryUnavailableException("Некорректные данные доставки");
    }
    int id() const { return deliveryId_; }
    DeliveryStatus status() const { return status_; }
    int courierId() const { return courierId_; }
    double fee() const { return fee_; }
    const std::string& createdAt() const { return createdAt_; }
    const DeliveryAddress& address() const { return address_; }
    bool assignCourier(Courier& courier) {
        if (status_ != DeliveryStatus::Created || !courier.isAvailable()) return false;
        if (!courier.assignDelivery()) return false;
        courierId_ = courier.id();
        status_ = DeliveryStatus::Assigned;
        return true;
    }
    void markPickedUp() {
        if (status_ != DeliveryStatus::Assigned) throw DeliveryUnavailableException("Курьер не назначен");
        status_ = DeliveryStatus::PickedUp;
    }
    void complete() {
        if (status_ != DeliveryStatus::PickedUp) throw DeliveryUnavailableException("Заказ ещё не передан курьеру");
        status_ = DeliveryStatus::Delivered;
    }
    void complete(Courier& courier) {
        if (courier.id() != courierId_) throw DeliveryUnavailableException("Курьер не соответствует доставке");
        complete();
        courier.completeDelivery();
    }
    void cancel() {
        if (status_ == DeliveryStatus::Delivered) throw DeliveryUnavailableException("Завершённую доставку нельзя отменить");
        status_ = DeliveryStatus::Cancelled;
    }
    bool isActive() const { return status_ == DeliveryStatus::Created || status_ == DeliveryStatus::Assigned || status_ == DeliveryStatus::PickedUp; }
    bool belongsToCustomer(int customerId) const { return customerId_ == customerId; }
};

} // namespace restaurant