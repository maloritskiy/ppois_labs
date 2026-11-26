#pragma once

#include "restaurant_orders.hpp"
#include "restaurant_staff.hpp"
#include "restaurant_exceptions.hpp"

#include <algorithm>
#include <string>
#include <vector>
#include <utility>

namespace restaurant {

enum class CookingStatus { Queued, InProgress, Ready, Cancelled };

class KitchenStation {
private:
    int stationId_ = 0;
    std::string name_;
    std::string supportedCategory_;
    bool busy_ = false;
    int currentTaskId_ = 0;

public:
    KitchenStation() = default;
    KitchenStation(int id, std::string name, std::string category)
        : stationId_(id), name_(std::move(name)), supportedCategory_(std::move(category)) {
        if (id <= 0 || name_.empty() || supportedCategory_.empty()) throw RestaurantException("Некорректная рабочая станция");
    }
    int id() const { return stationId_; }
    const std::string& name() const { return name_; }
    const std::string& category() const { return supportedCategory_; }
    bool isBusy() const { return busy_; }
    bool supports(const std::string& category) const { return supportedCategory_ == "all" || supportedCategory_ == category; }
    bool assignTask(int taskId, const std::string& category) {
        if (busy_ || taskId <= 0 || !supports(category)) return false;
        busy_ = true;
        currentTaskId_ = taskId;
        return true;
    }
    void releaseTask(int taskId) {
        if (!busy_ || currentTaskId_ != taskId) throw RestaurantException("На станции нет указанной задачи");
        busy_ = false;
        currentTaskId_ = 0;
    }
    bool handlesTask(int taskId) const { return busy_ && currentTaskId_ == taskId; }
};

class CookingTask {
private:
    int taskId_ = 0;
    int orderId_ = 0;
    int orderItemId_ = 0;
    std::string dishName_;
    std::string category_;
    int chefId_ = 0;
    int stationId_ = 0;
    CookingStatus status_ = CookingStatus::Queued;
    int estimatedMinutes_ = 0;

public:
    CookingTask() = default;
    CookingTask(int id, int orderId, int orderItemId, std::string dishName, int estimatedMinutes, std::string category = "all")
        : taskId_(id), orderId_(orderId), orderItemId_(orderItemId), dishName_(std::move(dishName)), category_(std::move(category)), estimatedMinutes_(estimatedMinutes) {
        if (id <= 0 || orderId <= 0 || orderItemId <= 0 || dishName_.empty() || estimatedMinutes < 0)
            throw RestaurantException("Некорректная задача кухни");
    }
    int id() const { return taskId_; }
    int orderId() const { return orderId_; }
    int orderItemId() const { return orderItemId_; }
    const std::string& dishName() const { return dishName_; }
    int chefId() const { return chefId_; }
    int stationId() const { return stationId_; }
    const std::string& category() const { return category_; }
    CookingStatus status() const { return status_; }
    bool assignChef(const Chef& chef) {
        if (!chef.isActive() || !chef.canPerform("cook") || status_ != CookingStatus::Queued) return false;
        chefId_ = chef.id();
        return true;
    }
    bool assignStation(const KitchenStation& station) {
        if (status_ != CookingStatus::Queued || station.isBusy() || !station.supports(dishName_)) return false;
        stationId_ = station.id();
        return true;
    }
    void start() {
        if (status_ != CookingStatus::Queued || chefId_ == 0 || stationId_ == 0)
            throw EmployeeNotAvailableException("Для задачи не назначены повар и рабочая станция");
        status_ = CookingStatus::InProgress;
    }
    void complete() {
        if (status_ != CookingStatus::InProgress) throw RestaurantException("Задача не находится в работе");
        status_ = CookingStatus::Ready;
    }
    void cancel() {
        if (status_ == CookingStatus::Ready || status_ == CookingStatus::Cancelled) throw RestaurantException("Готовую или отменённую задачу нельзя отменить");
        status_ = CookingStatus::Cancelled;
    }
    bool isOverdue(int elapsedMinutes) const { return status_ == CookingStatus::InProgress && elapsedMinutes > estimatedMinutes_; }
};

class Kitchen {
private:
    int kitchenId_ = 0;
    std::string name_;
    std::vector<KitchenStation> stations_;
    std::vector<CookingTask> tasks_;
    bool acceptingOrders_ = true;

public:
    Kitchen() = default;
    Kitchen(int id, std::string name) : kitchenId_(id), name_(std::move(name)) {
        if (id <= 0 || name_.empty()) throw RestaurantException("Некорректные данные кухни");
    }
    int id() const { return kitchenId_; }
    const std::string& name() const { return name_; }
    void addStation(const KitchenStation& station) {
        auto it = std::find_if(stations_.begin(), stations_.end(), [&station](const KitchenStation& item) { return item.id() == station.id(); });
        if (it != stations_.end()) throw RestaurantException("Рабочая станция уже существует");
        stations_.push_back(station);
    }
    void enqueueTask(const CookingTask& task) {
        if (!acceptingOrders_) throw RestaurantClosedException("Кухня не принимает новые задачи");
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [&task](const CookingTask& item) { return item.id() == task.id(); });
        if (it != tasks_.end()) throw RestaurantException("Задача уже добавлена");
        tasks_.push_back(task);
    }
    CookingTask* findTask(int taskId) {
        auto it = std::find_if(tasks_.begin(), tasks_.end(), [taskId](const CookingTask& item) { return item.id() == taskId; });
        return it == tasks_.end() ? nullptr : &*it;
    }
    KitchenStation* findStation(int stationId) {
        auto it = std::find_if(stations_.begin(), stations_.end(), [stationId](const KitchenStation& item) { return item.id() == stationId; });
        return it == stations_.end() ? nullptr : &*it;
    }
    bool assignTask(int taskId, const Chef& chef, int stationId) {
        CookingTask* task = findTask(taskId);
        KitchenStation* station = findStation(stationId);
        if (task == nullptr || station == nullptr) return false;
        if (!task->assignChef(chef) || !task->assignStation(*station)) return false;
        if (!station->assignTask(taskId, task->category())) return false;
        task->start();
        return true;
    }
    void finishTask(int taskId) {
        CookingTask* task = findTask(taskId);
        if (task == nullptr) throw RestaurantException("Задача не найдена");
        task->complete();
        KitchenStation* station = findStation(task->stationId());
        if (station != nullptr) station->releaseTask(taskId);
    }
    int countPendingTasks() const {
        return static_cast<int>(std::count_if(tasks_.begin(), tasks_.end(), [](const CookingTask& task) { return task.status() == CookingStatus::Queued; }));
    }
    void stopAcceptingOrders() { acceptingOrders_ = false; }
    void resumeAcceptingOrders() { acceptingOrders_ = true; }
};

} // namespace restaurant