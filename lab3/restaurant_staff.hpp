#pragma once

#include "restaurant_exceptions.hpp"

#include <algorithm>
#include <string>
#include <vector>
#include <utility>

namespace restaurant {

class Employee {
private:
    int employeeId_ = 0;
    std::string fullName_;
    std::string role_;
    double hourlyRate_ = 0.0;
    bool active_ = true;
    std::string hireDate_;

public:
    Employee() = default;
    Employee(int id, std::string name, std::string role, double rate, std::string hireDate)
        : employeeId_(id), fullName_(std::move(name)), role_(std::move(role)),
          hourlyRate_(rate), hireDate_(std::move(hireDate)) {
        if (id <= 0 || fullName_.empty() || role_.empty() || rate < 0.0)
            throw RestaurantException("Некорректные данные сотрудника");
    }
    virtual ~Employee() = default;

    int id() const { return employeeId_; }
    const std::string& name() const { return fullName_; }
    const std::string& role() const { return role_; }
    const std::string& hireDate() const { return hireDate_; }
    double hourlyRate() const { return hourlyRate_; }
    bool isActive() const { return active_; }
    virtual bool canPerform(const std::string& task) const { return active_ && task == role_; }
    void deactivate() { active_ = false; }
    void activate() { active_ = true; }
    void changeHourlyRate(double rate) {
        if (rate < 0.0) throw RestaurantException("Ставка не может быть отрицательной");
        hourlyRate_ = rate;
    }
    double calculatePay(int workedMinutes) const {
        if (workedMinutes < 0) throw RestaurantException("Рабочее время не может быть отрицательным");
        return hourlyRate_ * workedMinutes / 60.0;
    }
};

class Waiter : public Employee {
private:
    int assignedTables_ = 0;
    int completedOrders_ = 0;
    int maximumTables_ = 8;

public:
    Waiter() = default;
    Waiter(int id, std::string name, double rate, std::string hireDate)
        : Employee(id, std::move(name), "waiter", rate, std::move(hireDate)) {}
    bool canPerform(const std::string& task) const override {
        return isActive() && (task == "serve" || task == "take_order" || task == "close_bill");
    }
    bool assignTable() {
        if (!isActive() || assignedTables_ >= maximumTables_) return false;
        ++assignedTables_;
        return true;
    }
    void releaseTable() { if (assignedTables_ > 0) --assignedTables_; }
    void recordCompletedOrder() { ++completedOrders_; }
    int completedOrders() const { return completedOrders_; }
    int assignedTables() const { return assignedTables_; }
};

class Chef : public Employee {
private:
    std::vector<std::string> skills_;
    int completedTasks_ = 0;
    bool available_ = true;

public:
    Chef() = default;
    Chef(int id, std::string name, double rate, std::string hireDate)
        : Employee(id, std::move(name), "chef", rate, std::move(hireDate)) {}
    bool canPerform(const std::string& task) const override {
        return isActive() && available_ && (task == "cook" || std::find(skills_.begin(), skills_.end(), task) != skills_.end());
    }
    void addSkill(const std::string& skill) {
        if (!skill.empty() && std::find(skills_.begin(), skills_.end(), skill) == skills_.end()) skills_.push_back(skill);
    }
    bool hasSkill(const std::string& skill) const {
        return std::find(skills_.begin(), skills_.end(), skill) != skills_.end();
    }
    void setAvailable(bool value) { available_ = value; }
    void completeTask() { ++completedTasks_; available_ = true; }
    int completedTasks() const { return completedTasks_; }
};

class Administrator : public Employee {
private:
    int managedEmployees_ = 0;
    bool canManageFinance_ = true;

public:
    Administrator() = default;
    Administrator(int id, std::string name, double rate, std::string hireDate)
        : Employee(id, std::move(name), "administrator", rate, std::move(hireDate)) {}
    bool canPerform(const std::string& task) const override {
        return isActive() && (task == "manage_staff" || task == "manage_menu" || task == "manage_orders" ||
                              (task == "manage_finance" && canManageFinance_));
    }
    void addManagedEmployee() { ++managedEmployees_; }
    void removeManagedEmployee() { if (managedEmployees_ > 0) --managedEmployees_; }
    void setFinancePermission(bool value) { canManageFinance_ = value; }
    int managedEmployees() const { return managedEmployees_; }
};

class WorkShift {
private:
    int shiftId_ = 0;
    int employeeId_ = 0;
    std::string date_;
    int startMinute_ = 0;
    int endMinute_ = 0;
    bool completed_ = false;

public:
    WorkShift() = default;
    WorkShift(int id, int employeeId, std::string date, int start, int end)
        : shiftId_(id), employeeId_(employeeId), date_(std::move(date)), startMinute_(start), endMinute_(end) {
        if (id <= 0 || employeeId <= 0 || date_.empty() || start < 0 || end > 1440 || end <= start)
            throw RestaurantException("Некорректная рабочая смена");
    }
    int id() const { return shiftId_; }
    int employeeId() const { return employeeId_; }
    const std::string& date() const { return date_; }
    int durationMinutes() const { return endMinute_ - startMinute_; }
    int startMinute() const { return startMinute_; }
    int endMinute() const { return endMinute_; }
    bool overlaps(const WorkShift& other) const {
        return employeeId_ == other.employeeId_ && date_ == other.date_ &&
               startMinute_ < other.endMinute_ && other.startMinute_ < endMinute_;
    }
    bool includesMinute(int minute) const { return minute >= startMinute_ && minute < endMinute_; }
    void markCompleted() { completed_ = true; }
    bool isCompleted() const { return completed_; }
};

class WorkSchedule {
private:
    std::vector<WorkShift> shifts_;
    int weeklyLimitMinutes_ = 2400;
    int minimumRestMinutes_ = 600;

public:
    WorkSchedule() = default;
    void addShift(const WorkShift& shift) {
        for (const auto& existing : shifts_) {
            if (existing.id() == shift.id()) throw ShiftConflictException("Идентификатор смены уже используется");
            if (existing.overlaps(shift)) throw ShiftConflictException("Смены сотрудника пересекаются");
        }
        if (!hasRestPeriod(shift)) throw ShiftConflictException("Между сменами недостаточно времени для отдыха");
        shifts_.push_back(shift);
    }
    void removeShift(int shiftId) {
        auto it = std::remove_if(shifts_.begin(), shifts_.end(), [shiftId](const WorkShift& item) { return item.id() == shiftId; });
        if (it == shifts_.end()) throw RestaurantException("Смена не найдена");
        shifts_.erase(it, shifts_.end());
    }
    const WorkShift* findShift(int shiftId) const {
        auto it = std::find_if(shifts_.begin(), shifts_.end(), [shiftId](const WorkShift& item) { return item.id() == shiftId; });
        return it == shifts_.end() ? nullptr : &*it;
    }
    int totalMinutesForEmployee(int employeeId) const {
        int total = 0;
        for (const auto& shift : shifts_) if (shift.employeeId() == employeeId) total += shift.durationMinutes();
        return total;
    }
    bool isWithinWeeklyLimit(int employeeId, int additionalMinutes) const {
        return additionalMinutes >= 0 && totalMinutesForEmployee(employeeId) + additionalMinutes <= weeklyLimitMinutes_;
    }
    void setWeeklyLimit(int minutes) {
        if (minutes <= 0) throw RestaurantException("Лимит рабочего времени должен быть положительным");
        weeklyLimitMinutes_ = minutes;
    }
    bool hasRestPeriod(const WorkShift& newShift) const {
        for (const auto& shift : shifts_) {
            if (shift.employeeId() != newShift.employeeId() || shift.date() != newShift.date()) continue;
            int restMinutes = 0;
            if (shift.endMinute() <= newShift.startMinute()) {
                restMinutes = newShift.startMinute() - shift.endMinute();
            } else if (newShift.endMinute() <= shift.startMinute()) {
                restMinutes = shift.startMinute() - newShift.endMinute();
            }
            if (restMinutes < minimumRestMinutes_) return false;
        }
        return true;
    }
    const std::vector<WorkShift>& shifts() const { return shifts_; }
};

class AttendanceRecord {
private:
    int recordId_ = 0;
    int employeeId_ = 0;
    int shiftId_ = 0;
    int checkInMinute_ = -1;
    int checkOutMinute_ = -1;
    std::string note_;

public:
    AttendanceRecord() = default;
    AttendanceRecord(int id, int employeeId, int shiftId)
        : recordId_(id), employeeId_(employeeId), shiftId_(shiftId) {
        if (id <= 0 || employeeId <= 0 || shiftId <= 0) throw RestaurantException("Некорректная запись посещаемости");
    }
    void clockIn(int minute) {
        if (minute < 0 || minute >= 1440 || checkInMinute_ != -1) throw RestaurantException("Некорректная отметка прихода");
        checkInMinute_ = minute;
    }
    void clockOut(int minute) {
        if (minute < 0 || minute >= 1440 || checkInMinute_ == -1 || minute < checkInMinute_ || checkOutMinute_ != -1)
            throw RestaurantException("Некорректная отметка ухода");
        checkOutMinute_ = minute;
    }
    int workedMinutes() const { return checkInMinute_ == -1 || checkOutMinute_ == -1 ? 0 : checkOutMinute_ - checkInMinute_; }
    bool isLate(int scheduledStartMinute) const { return checkInMinute_ != -1 && checkInMinute_ > scheduledStartMinute; }
    bool isComplete() const { return checkInMinute_ != -1 && checkOutMinute_ != -1; }
    void addNote(const std::string& note) { note_ = note; }
    int id() const { return recordId_; }
    int employeeId() const { return employeeId_; }
    int shiftId() const { return shiftId_; }
    const std::string& note() const { return note_; }
};

} // namespace restaurant