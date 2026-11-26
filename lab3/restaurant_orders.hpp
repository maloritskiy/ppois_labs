#pragma once

#include "restaurant_customers.hpp"
#include "restaurant_staff.hpp"
#include "restaurant_exceptions.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>
#include <utility>

namespace restaurant {

enum class OrderStatus { Created, Accepted, Preparing, Ready, Served, Paid, Cancelled };

class OrderItem {
private:
    MenuItem menuItem_;
    int quantity_ = 1;
    double unitPrice_ = 0.0;
    std::string note_;

public:
    OrderItem() = default;
    OrderItem(const MenuItem& item, int quantity, std::string note = {})
        : menuItem_(item), quantity_(quantity), unitPrice_(item.price()), note_(std::move(note)) {
        if (!item.canBeOrdered() || quantity <= 0) throw InvalidOrderException("Некорректная позиция заказа");
    }
    int itemId() const { return menuItem_.id(); }
    const std::string& name() const { return menuItem_.name(); }
    const std::string& note() const { return note_; }
    int quantity() const { return quantity_; }
    double unitPrice() const { return unitPrice_; }
    double lineTotal() const { return unitPrice_ * quantity_; }
    void changeQuantity(int quantity) {
        if (quantity <= 0) throw InvalidOrderException("Количество должно быть положительным");
        quantity_ = quantity;
    }
    void changeNote(const std::string& note) { note_ = note; }
    bool refersTo(const MenuItem& item) const { return itemId() == item.id(); }
};

class OrderStatusHistory {
private:
    OrderStatus status_ = OrderStatus::Created;
    std::string timestamp_;
    std::string note_;
    int employeeId_ = 0;

public:
    OrderStatusHistory() = default;
    OrderStatusHistory(OrderStatus status, std::string timestamp, std::string note, int employeeId)
        : status_(status), timestamp_(std::move(timestamp)), note_(std::move(note)), employeeId_(employeeId) {}
    OrderStatus status() const { return status_; }
    const std::string& timestamp() const { return timestamp_; }
    bool changedBy(int employeeId) const { return employeeId_ == employeeId; }
    bool hasNote() const { return !note_.empty(); }
};

class Order {
private:
    int orderId_ = 0;
    Customer customer_;
    int tableId_ = 0;
    int reservationId_ = 0;
    std::vector<OrderItem> items_;
    std::vector<OrderStatusHistory> history_;
    OrderStatus status_ = OrderStatus::Created;
    std::string createdAt_;
    int waiterId_ = 0;

public:
    Order() = default;
    Order(int id, const Customer& customer, int tableId, std::string createdAt)
        : orderId_(id), customer_(customer), tableId_(tableId), createdAt_(std::move(createdAt)) {
        if (id <= 0 || !customer.isActive() || tableId < 0 || createdAt_.empty())
            throw InvalidOrderException("Некорректные данные заказа");
    }
    int id() const { return orderId_; }
    int customerId() const { return customer_.id(); }
    int tableId() const { return tableId_; }
    int reservationId() const { return reservationId_; }
    OrderStatus status() const { return status_; }
    const std::vector<OrderItem>& items() const { return items_; }
    const std::string& createdAt() const { return createdAt_; }
    void assignWaiter(const Waiter& waiter) {
        if (!waiter.isActive()) throw EmployeeNotAvailableException("Официант неактивен");
        waiterId_ = waiter.id();
    }
    int waiterId() const { return waiterId_; }
    void attachReservation(const Reservation& reservation) {
        if (status_ != OrderStatus::Created || reservationId_ != 0 ||
            !reservation.isForCustomer(customer_.id()) ||
            reservation.status() != ReservationStatus::Confirmed ||
            (tableId_ != 0 && tableId_ != reservation.tableId()))
            throw InvalidOrderException("Бронирование нельзя связать с этим заказом");
        reservationId_ = reservation.id();
        tableId_ = reservation.tableId();
    }
    void addItem(const MenuItem& item, int quantity, const std::string& note = {}) {
        if (status_ == OrderStatus::Paid || status_ == OrderStatus::Cancelled) throw InvalidOrderException("Нельзя изменять закрытый заказ");
        auto it = std::find_if(items_.begin(), items_.end(), [&item](const OrderItem& row) { return row.itemId() == item.id(); });
        if (it == items_.end()) items_.emplace_back(item, quantity, note);
        else it->changeQuantity(it->quantity() + quantity);
    }
    void removeItem(int itemId) {
        auto it = std::remove_if(items_.begin(), items_.end(), [itemId](const OrderItem& item) { return item.itemId() == itemId; });
        if (it == items_.end()) throw DishNotFoundException("Позиция заказа не найдена");
        items_.erase(it, items_.end());
    }
    void changeItemQuantity(int itemId, int quantity) {
        auto it = std::find_if(items_.begin(), items_.end(), [itemId](const OrderItem& item) { return item.itemId() == itemId; });
        if (it == items_.end()) throw DishNotFoundException("Позиция заказа не найдена");
        it->changeQuantity(quantity);
    }
    double calculateTotal() const {
        double total = 0.0;
        for (const auto& item : items_) total += item.lineTotal();
        return total;
    }
    void changeStatus(OrderStatus newStatus, const std::string& timestamp, int employeeId = 0) {
        if (items_.empty() && newStatus != OrderStatus::Cancelled) throw InvalidOrderException("Нельзя изменить статус пустого заказа");
        if (status_ == OrderStatus::Paid || status_ == OrderStatus::Cancelled) throw InvalidOrderException("Заказ уже закрыт");
        if (newStatus == OrderStatus::Created) throw InvalidOrderException("Нельзя вернуть заказ в начальное состояние");
        status_ = newStatus;
        history_.emplace_back(status_, timestamp, "Изменение статуса", employeeId);
    }
    void cancel(const std::string& timestamp) {
        if (status_ == OrderStatus::Paid || status_ == OrderStatus::Served) throw InvalidOrderException("Нельзя отменить оплаченный или обслуженный заказ");
        status_ = OrderStatus::Cancelled;
        history_.emplace_back(status_, timestamp, "Заказ отменён", 0);
    }
    bool containsItem(int itemId) const {
        return std::any_of(items_.begin(), items_.end(), [itemId](const OrderItem& item) { return item.itemId() == itemId; });
    }
    const std::vector<OrderStatusHistory>& history() const { return history_; }
};

class BillLine {
private:
    int itemId_ = 0;
    std::string description_;
    int quantity_ = 0;
    double unitPrice_ = 0.0;

public:
    BillLine() = default;
    BillLine(int itemId, std::string description, int quantity, double unitPrice)
        : itemId_(itemId), description_(std::move(description)), quantity_(quantity), unitPrice_(unitPrice) {
        if (itemId <= 0 || description_.empty() || quantity <= 0 || unitPrice < 0.0) throw RestaurantException("Некорректная строка счёта");
    }
    int itemId() const { return itemId_; }
    const std::string& description() const { return description_; }
    double total() const { return quantity_ * unitPrice_; }
    void changeQuantity(int quantity) {
        if (quantity <= 0) throw RestaurantException("Количество должно быть положительным");
        quantity_ = quantity;
    }
    bool hasSameItem(const BillLine& other) const { return itemId_ == other.itemId_; }
};

class Bill {
private:
    int billId_ = 0;
    int orderId_ = 0;
    std::vector<BillLine> lines_;
    double serviceChargePercent_ = 0.0;
    double discountAmount_ = 0.0;
    bool closed_ = false;

public:
    Bill() = default;
    Bill(int id, const Order& order) : billId_(id), orderId_(order.id()) {
        if (id <= 0 || order.items().empty()) throw RestaurantException("Нельзя создать счёт для пустого заказа");
        for (const auto& item : order.items()) lines_.emplace_back(item.itemId(), item.name(), item.quantity(), item.unitPrice());
    }
    int id() const { return billId_; }
    int orderId() const { return orderId_; }
    const std::vector<BillLine>& lines() const { return lines_; }
    double subtotal() const {
        double result = 0.0;
        for (const auto& line : lines_) result += line.total();
        return result;
    }
    void setServiceCharge(double percent) {
        if (percent < 0.0 || percent > 100.0) throw RestaurantException("Некорректная сервисная надбавка");
        serviceChargePercent_ = percent;
    }
    void applyDiscount(double amount) {
        if (amount < 0.0 || amount > subtotal()) throw RestaurantException("Некорректная скидка");
        discountAmount_ = amount;
    }
    double calculateTotal() const { return subtotal() * (1.0 + serviceChargePercent_ / 100.0) - discountAmount_; }
    void close() { closed_ = true; }
    bool isClosed() const { return closed_; }
};

class Payment {
protected:
    int paymentId_ = 0;
    double amount_ = 0.0;
    bool processed_ = false;
    std::string method_;

public:
    Payment() = default;
    Payment(int id, double amount, std::string method)
        : paymentId_(id), amount_(amount), method_(std::move(method)) {
        if (id <= 0 || amount <= 0.0 || method_.empty()) throw PaymentFailedException("Некорректные данные платежа");
    }
    virtual ~Payment() = default;
    int id() const { return paymentId_; }
    double amount() const { return amount_; }
    bool processed() const { return processed_; }
    virtual bool process() { processed_ = amount_ > 0.0; return processed_; }
    virtual std::string method() const { return method_; }
    void reverse() { if (!processed_) throw PaymentFailedException("Платёж ещё не выполнен"); processed_ = false; }
};

class CashPayment : public Payment {
private:
    double cashReceived_ = 0.0;
    double change_ = 0.0;

public:
    CashPayment() = default;
    CashPayment(int id, double amount, double received)
        : Payment(id, amount, "cash"), cashReceived_(received) {
        if (received < amount) throw PaymentFailedException("Недостаточно наличных средств");
    }
    bool process() override {
        if (cashReceived_ < amount_) return false;
        change_ = cashReceived_ - amount_;
        processed_ = true;
        return true;
    }
    double change() const { return change_; }
    void addCash(double amount) {
        if (amount < 0.0) throw PaymentFailedException("Сумма не может быть отрицательной");
        cashReceived_ += amount;
    }
};

class CardPayment : public Payment {
private:
    std::string maskedCardNumber_;
    std::string authorizationCode_;
    bool approved_ = false;

public:
    CardPayment() = default;
    CardPayment(int id, double amount, std::string maskedCard)
        : Payment(id, amount, "card"), maskedCardNumber_(std::move(maskedCard)) {
        if (maskedCardNumber_.empty()) throw PaymentFailedException("Не указан номер карты");
    }
    void approve(const std::string& code) { approved_ = !code.empty(); authorizationCode_ = code; }
    bool process() override {
        processed_ = approved_;
        return processed_;
    }
    bool isApproved() const { return approved_; }
    const std::string& authorizationCode() const { return authorizationCode_; }
    std::string method() const override { return "card"; }
};

class Refund {
private:
    int refundId_ = 0;
    int paymentId_ = 0;
    double amount_ = 0.0;
    std::string reason_;
    bool completed_ = false;

public:
    Refund() = default;
    Refund(int id, const Payment& payment, double amount, std::string reason)
        : refundId_(id), paymentId_(payment.id()), amount_(amount), reason_(std::move(reason)) {
        if (id <= 0 || !payment.processed() || amount <= 0.0 || amount > payment.amount() || reason_.empty())
            throw PaymentFailedException("Некорректные данные возврата");
    }
    bool complete() { if (completed_) return false; completed_ = true; return true; }
    bool isCompleted() const { return completed_; }
    double amount() const { return amount_; }
    const std::string& reason() const { return reason_; }
    bool belongsToPayment(int paymentId) const { return paymentId_ == paymentId; }
};

class Discount {
private:
    int discountId_ = 0;
    std::string name_;
    double percentage_ = 0.0;
    double minimumTotal_ = 0.0;
    bool active_ = true;

public:
    Discount() = default;
    Discount(int id, std::string name, double percentage, double minimumTotal)
        : discountId_(id), name_(std::move(name)), percentage_(percentage), minimumTotal_(minimumTotal) {
        if (id <= 0 || name_.empty() || percentage < 0.0 || percentage > 100.0 || minimumTotal < 0.0)
            throw RestaurantException("Некорректные параметры скидки");
    }
    double calculateDiscount(double total) const {
        if (!active_ || total < minimumTotal_ || total < 0.0) return 0.0;
        return total * percentage_ / 100.0;
    }
    void activate() { active_ = true; }
    void deactivate() { active_ = false; }
    bool appliesTo(double total) const { return active_ && total >= minimumTotal_; }
    double percentage() const { return percentage_; }
    const std::string& name() const { return name_; }
};

class Coupon {
private:
    std::string code_;
    Discount discount_;
    std::string expiryDate_;
    int usesRemaining_ = 0;
    bool active_ = true;

public:
    Coupon() = default;
    Coupon(std::string code, const Discount& discount, std::string expiryDate, int uses)
        : code_(std::move(code)), discount_(discount), expiryDate_(std::move(expiryDate)), usesRemaining_(uses) {
        if (code_.empty() || expiryDate_.empty() || uses < 0) throw RestaurantException("Некорректный купон");
    }
    bool isValid(const std::string& date) const { return active_ && usesRemaining_ > 0 && date <= expiryDate_; }
    double calculateDiscount(double total, const std::string& date) const {
        return isValid(date) ? discount_.calculateDiscount(total) : 0.0;
    }
    bool use(const std::string& date) {
        if (!isValid(date)) return false;
        --usesRemaining_;
        return true;
    }
    void deactivate() { active_ = false; }
    const std::string& code() const { return code_; }
};

} // namespace restaurant