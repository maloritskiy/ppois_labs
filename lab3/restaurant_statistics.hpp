#pragma once

#include "restaurant_delivery.hpp"
#include "restaurant_inventory.hpp"
#include "restaurant_kitchen.hpp"
#include "restaurant_orders.hpp"
#include "restaurant_staff.hpp"

#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <utility>

namespace restaurant {

class SalesReport {
private:
    std::string periodStart_;
    std::string periodEnd_;
    int orderCount_ = 0;
    double grossRevenue_ = 0.0;
    double discounts_ = 0.0;
    double refunds_ = 0.0;

public:
    SalesReport() = default;
    SalesReport(std::string start, std::string end)
        : periodStart_(std::move(start)), periodEnd_(std::move(end)) {
        if (periodStart_.empty() || periodEnd_.empty() || periodEnd_ < periodStart_)
            throw RestaurantException("Некорректный период отчёта");
    }
    void recordSale(const Bill& bill) {
        if (bill.calculateTotal() < 0.0) throw RestaurantException("Некорректный счёт");
        ++orderCount_;
        grossRevenue_ += bill.subtotal();
        discounts_ += bill.subtotal() - bill.calculateTotal();
    }
    void recordRefund(const Refund& refund) { refunds_ += refund.amount(); }
    double netRevenue() const { return grossRevenue_ - discounts_ - refunds_; }
    double averageCheck() const { return orderCount_ == 0 ? 0.0 : netRevenue() / orderCount_; }
    int orderCount() const { return orderCount_; }
    bool hasSales() const { return orderCount_ > 0; }
    const std::string& periodStart() const { return periodStart_; }
    const std::string& periodEnd() const { return periodEnd_; }
};

class InventoryReport {
private:
    std::string generatedAt_;
    int itemCount_ = 0;
    int lowStockCount_ = 0;
    double totalValue_ = 0.0;
    double totalQuantity_ = 0.0;

public:
    InventoryReport() = default;
    explicit InventoryReport(std::string generatedAt) : generatedAt_(std::move(generatedAt)) {}
    void updateFrom(const Inventory& inventory) {
        itemCount_ = static_cast<int>(inventory.items().size());
        lowStockCount_ = static_cast<int>(inventory.itemsToRestock().size());
        totalValue_ = inventory.calculateTotalValue();
        totalQuantity_ = inventory.usedCapacity();
    }
    bool hasLowStock() const { return lowStockCount_ > 0; }
    double averageItemValue() const { return itemCount_ == 0 ? 0.0 : totalValue_ / itemCount_; }
    int itemCount() const { return itemCount_; }
    int lowStockCount() const { return lowStockCount_; }
    double totalValue() const { return totalValue_; }
    double totalQuantity() const { return totalQuantity_; }
    const std::string& generatedAt() const { return generatedAt_; }
};

class MenuPerformanceReport {
private:
    std::string period_;
    std::map<int, int> soldItems_;
    int totalItemsSold_ = 0;
    double revenue_ = 0.0;
    int unavailableEvents_ = 0;

public:
    MenuPerformanceReport() = default;
    explicit MenuPerformanceReport(std::string period) : period_(std::move(period)) {}
    void recordSale(const MenuItem& item, int quantity) {
        if (quantity <= 0) throw RestaurantException("Количество продаж должно быть положительным");
        soldItems_[item.id()] += quantity;
        totalItemsSold_ += quantity;
        revenue_ += item.price() * quantity;
    }
    void recordUnavailableEvent() { ++unavailableEvents_; }
    int quantitySold(int itemId) const {
        auto it = soldItems_.find(itemId);
        return it == soldItems_.end() ? 0 : it->second;
    }
    int popularItemId() const {
        if (soldItems_.empty()) return 0;
        return std::max_element(soldItems_.begin(), soldItems_.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; })->first;
    }
    double revenue() const { return revenue_; }
    int totalItemsSold() const { return totalItemsSold_; }
    int unavailableEvents() const { return unavailableEvents_; }
    const std::string& period() const { return period_; }
};

class StaffPerformanceReport {
private:
    std::string period_;
    std::map<int, int> completedTasks_;
    std::map<int, int> workedMinutes_;
    int lateArrivals_ = 0;
    int absences_ = 0;

public:
    StaffPerformanceReport() = default;
    explicit StaffPerformanceReport(std::string period) : period_(std::move(period)) {}
    void recordTask(const Chef& chef) { ++completedTasks_[chef.id()]; }
    void recordWorkedTime(const Employee& employee, int minutes) {
        if (minutes < 0) throw RestaurantException("Рабочее время не может быть отрицательным");
        workedMinutes_[employee.id()] += minutes;
    }
    void recordLateArrival() { ++lateArrivals_; }
    void recordAbsence() { ++absences_; }
    int tasksCompleted(int employeeId) const {
        auto it = completedTasks_.find(employeeId);
        return it == completedTasks_.end() ? 0 : it->second;
    }
    int workedMinutes(int employeeId) const {
        auto it = workedMinutes_.find(employeeId);
        return it == workedMinutes_.end() ? 0 : it->second;
    }
    int punctualityIncidents() const { return lateArrivals_ + absences_; }
    bool hasRecords() const { return !completedTasks_.empty() || !workedMinutes_.empty(); }
    const std::string& period() const { return period_; }
};

class RestaurantStatistics {
private:
    int totalOrders_ = 0;
    int completedOrders_ = 0;
    int cancelledOrders_ = 0;
    int totalCustomers_ = 0;
    int completedDeliveries_ = 0;
    double totalRevenue_ = 0.0;
    double totalRefunds_ = 0.0;

public:
    RestaurantStatistics() = default;
    void recordOrder(const Order& order) {
        ++totalOrders_;
        if (order.status() == OrderStatus::Cancelled) ++cancelledOrders_;
        if (order.status() == OrderStatus::Served || order.status() == OrderStatus::Paid) ++completedOrders_;
    }
    void recordCustomer(const Customer& customer) { if (customer.isActive()) ++totalCustomers_; }
    void recordPayment(const Payment& payment) {
        if (payment.processed()) totalRevenue_ += payment.amount();
    }
    void recordRefund(const Refund& refund) { if (refund.isCompleted()) totalRefunds_ += refund.amount(); }
    void recordDelivery(const Delivery& delivery) {
        if (delivery.status() == DeliveryStatus::Delivered) ++completedDeliveries_;
    }
    double netRevenue() const { return totalRevenue_ - totalRefunds_; }
    double averageOrderValue() const { return totalOrders_ == 0 ? 0.0 : totalRevenue_ / totalOrders_; }
    double cancellationRate() const { return totalOrders_ == 0 ? 0.0 : static_cast<double>(cancelledOrders_) / totalOrders_; }
    int totalOrders() const { return totalOrders_; }
    int completedOrders() const { return completedOrders_; }
    int completedDeliveries() const { return completedDeliveries_; }
};

} // namespace restaurant