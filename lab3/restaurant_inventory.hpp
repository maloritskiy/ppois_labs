#pragma once

#include "restaurant_menu.hpp"
#include "restaurant_exceptions.hpp"

#include <algorithm>
#include <string>
#include <vector>
#include <cstddef>
#include <utility>

namespace restaurant {

enum class MovementType { Receipt, Consumption, Waste, Correction };

class StockItem {
private:
    Ingredient ingredient_;
    double quantity_ = 0.0;
    double minimumQuantity_ = 0.0;
    double reservedQuantity_ = 0.0;
    std::string storageLocation_;
    std::string lotNumber_;

public:
    StockItem() = default;
    StockItem(const Ingredient& ingredient, double quantity, double minimumQuantity,
              std::string location, std::string lot)
        : ingredient_(ingredient), quantity_(quantity), minimumQuantity_(minimumQuantity),
          storageLocation_(std::move(location)), lotNumber_(std::move(lot)) {
        if (quantity < 0.0 || minimumQuantity < 0.0 || storageLocation_.empty() || lotNumber_.empty())
            throw RestaurantException("Некорректная складская позиция");
    }
    int ingredientId() const { return ingredient_.id(); }
    const Ingredient& ingredient() const { return ingredient_; }
    const std::string& storageLocation() const { return storageLocation_; }
    const std::string& lotNumber() const { return lotNumber_; }
    double quantity() const { return quantity_; }
    double availableQuantity() const { return quantity_ - reservedQuantity_; }
    bool receive(double amount) {
        if (amount <= 0.0) return false;
        quantity_ += amount;
        return true;
    }
    bool consume(double amount) {
        if (amount <= 0.0 || amount > availableQuantity()) return false;
        quantity_ -= amount;
        return true;
    }
    bool reserve(double amount) {
        if (amount <= 0.0 || amount > availableQuantity()) return false;
        reservedQuantity_ += amount;
        return true;
    }
    void releaseReservation(double amount) {
        if (amount <= 0.0 || amount > reservedQuantity_) throw InsufficientStockException("Некорректное снятие резерва");
        reservedQuantity_ -= amount;
    }
    bool needsRestock() const { return availableQuantity() <= minimumQuantity_; }
    double totalValue() const { return quantity_ * ingredient_.purchasePrice(); }
    bool isSameIngredient(const Ingredient& ingredient) const { return ingredientId() == ingredient.id(); }
};

class StockMovement {
private:
    int movementId_ = 0;
    int ingredientId_ = 0;
    double quantity_ = 0.0;
    MovementType type_ = MovementType::Receipt;
    std::string reason_;
    std::string date_;

public:
    StockMovement() = default;
    StockMovement(int id, int ingredientId, double quantity, MovementType type,
                  std::string reason, std::string date)
        : movementId_(id), ingredientId_(ingredientId), quantity_(quantity), type_(type),
          reason_(std::move(reason)), date_(std::move(date)) {
        if (id <= 0 || ingredientId <= 0 || quantity <= 0.0 || date_.empty())
            throw RestaurantException("Некорректное движение товара");
    }
    int id() const { return movementId_; }
    int ingredientId() const { return ingredientId_; }
    double quantity() const { return quantity_; }
    bool isIncoming() const { return type_ == MovementType::Receipt || (type_ == MovementType::Correction && reason_ == "incoming"); }
    double signedQuantity() const { return isIncoming() ? quantity_ : -quantity_; }
    bool isWaste() const { return type_ == MovementType::Waste; }
    const std::string& reason() const { return reason_; }
    const std::string& date() const { return date_; }
    bool concernsIngredient(const Ingredient& ingredient) const { return ingredient.id() == ingredientId_; }
};

class Inventory {
private:
    int inventoryId_ = 0;
    std::string name_;
    std::vector<StockItem> items_;
    std::vector<StockMovement> movements_;
    double totalCapacity_ = 0.0;

public:
    Inventory() = default;
    Inventory(int id, std::string name, double capacity)
        : inventoryId_(id), name_(std::move(name)), totalCapacity_(capacity) {
        if (id <= 0 || name_.empty() || capacity <= 0.0) throw RestaurantException("Некорректные данные склада");
    }
    int id() const { return inventoryId_; }
    const std::string& name() const { return name_; }
    void addStockItem(const StockItem& item) {
        if (findItem(item.ingredientId()) != nullptr) throw RestaurantException("Ингредиент уже есть на складе");
        items_.push_back(item);
    }
    StockItem* findItem(int ingredientId) {
        auto it = std::find_if(items_.begin(), items_.end(), [ingredientId](const StockItem& item) { return item.ingredientId() == ingredientId; });
        return it == items_.end() ? nullptr : &*it;
    }
    const StockItem* findItem(int ingredientId) const {
        auto it = std::find_if(items_.begin(), items_.end(), [ingredientId](const StockItem& item) { return item.ingredientId() == ingredientId; });
        return it == items_.end() ? nullptr : &*it;
    }
    void removeStockItem(int ingredientId) {
        auto it = std::remove_if(items_.begin(), items_.end(), [ingredientId](const StockItem& item) { return item.ingredientId() == ingredientId; });
        if (it == items_.end()) throw IngredientUnavailableException("Складская позиция не найдена");
        items_.erase(it, items_.end());
    }
    bool receive(int ingredientId, double quantity, const StockMovement& movement) {
        StockItem* item = findItem(ingredientId);
        if (item == nullptr || movement.ingredientId() != ingredientId || !movement.isIncoming()) return false;
        if (!isCapacityAvailable(quantity) || !item->receive(quantity)) return false;
        movements_.push_back(movement);
        return true;
    }
    bool consume(int ingredientId, double quantity, const StockMovement& movement) {
        StockItem* item = findItem(ingredientId);
        if (item == nullptr) throw IngredientUnavailableException("Ингредиент отсутствует на складе");
        if (movement.ingredientId() != ingredientId || movement.isIncoming()) return false;
        if (!item->consume(quantity)) throw InsufficientStockException("Недостаточно ингредиента на складе");
        movements_.push_back(movement);
        return true;
    }
    std::vector<int> itemsToRestock() const {
        std::vector<int> result;
        for (const auto& item : items_) if (item.needsRestock()) result.push_back(item.ingredientId());
        return result;
    }
    double calculateTotalValue() const {
        double total = 0.0;
        for (const auto& item : items_) total += item.totalValue();
        return total;
    }
    double usedCapacity() const {
        double used = 0.0;
        for (const auto& item : items_) used += item.quantity();
        return used;
    }
    bool isCapacityAvailable(double additionalQuantity) const {
        return additionalQuantity >= 0.0 && usedCapacity() + additionalQuantity <= totalCapacity_;
    }
    const std::vector<StockItem>& items() const { return items_; }
    const std::vector<StockMovement>& movements() const { return movements_; }
};

class Supplier {
private:
    int supplierId_ = 0;
    std::string name_;
    std::string contactName_;
    std::string phoneNumber_;
    std::vector<int> ingredientIds_;
    bool active_ = true;

public:
    Supplier() = default;
    Supplier(int id, std::string name, std::string contact, std::string phone)
        : supplierId_(id), name_(std::move(name)), contactName_(std::move(contact)), phoneNumber_(std::move(phone)) {
        if (id <= 0 || name_.empty()) throw RestaurantException("Некорректные данные поставщика");
    }
    int id() const { return supplierId_; }
    const std::string& name() const { return name_; }
    const std::string& contactName() const { return contactName_; }
    const std::string& phoneNumber() const { return phoneNumber_; }
    void addIngredient(const Ingredient& ingredient) {
        if (std::find(ingredientIds_.begin(), ingredientIds_.end(), ingredient.id()) == ingredientIds_.end()) ingredientIds_.push_back(ingredient.id());
    }
    bool supplies(const Ingredient& ingredient) const {
        return std::find(ingredientIds_.begin(), ingredientIds_.end(), ingredient.id()) != ingredientIds_.end();
    }
    void deactivate() { active_ = false; }
    void activate() { active_ = true; }
    bool isActive() const { return active_; }
    void updateContact(std::string contact, std::string phone) {
        contactName_ = std::move(contact);
        phoneNumber_ = std::move(phone);
    }
};

class PurchaseOrderItem {
private:
    int ingredientId_ = 0;
    double quantity_ = 0.0;
    double unitPrice_ = 0.0;
    double receivedQuantity_ = 0.0;

public:
    PurchaseOrderItem() = default;
    PurchaseOrderItem(const Ingredient& ingredient, double quantity, double unitPrice)
        : ingredientId_(ingredient.id()), quantity_(quantity), unitPrice_(unitPrice) {
        if (quantity <= 0.0 || unitPrice < 0.0) throw RestaurantException("Некорректная позиция закупки");
    }
    int ingredientId() const { return ingredientId_; }
    double quantity() const { return quantity_; }
    double receivedQuantity() const { return receivedQuantity_; }
    double totalCost() const { return quantity_ * unitPrice_; }
    double pendingQuantity() const { return quantity_ - receivedQuantity_; }
    void receive(double quantity) {
        if (quantity <= 0.0 || receivedQuantity_ + quantity > quantity_) throw RestaurantException("Некорректное количество приёмки");
        receivedQuantity_ += quantity;
    }
    bool isComplete() const { return receivedQuantity_ >= quantity_; }
};

enum class PurchaseOrderStatus { Draft, Sent, PartiallyReceived, Received, Cancelled };

class PurchaseOrder {
private:
    int purchaseOrderId_ = 0;
    Supplier supplier_;
    std::vector<PurchaseOrderItem> items_;
    std::string orderDate_;
    PurchaseOrderStatus status_ = PurchaseOrderStatus::Draft;
    std::string note_;

public:
    PurchaseOrder() = default;
    PurchaseOrder(int id, const Supplier& supplier, std::string date)
        : purchaseOrderId_(id), supplier_(supplier), orderDate_(std::move(date)) {
        if (id <= 0 || !supplier.isActive() || orderDate_.empty()) throw SupplierNotFoundException("Поставщик недоступен или данные заказа некорректны");
    }
    int id() const { return purchaseOrderId_; }
    const std::string& orderDate() const { return orderDate_; }
    const std::string& note() const { return note_; }
    PurchaseOrderStatus status() const { return status_; }
    void addItem(const PurchaseOrderItem& item) {
        if (status_ != PurchaseOrderStatus::Draft) throw RestaurantException("Нельзя изменять отправленную закупку");
        items_.push_back(item);
    }
    double calculateTotal() const {
        double total = 0.0;
        for (const auto& item : items_) total += item.totalCost();
        return total;
    }
    void send() {
        if (status_ != PurchaseOrderStatus::Draft || items_.empty()) throw RestaurantException("Закупка не готова к отправке");
        status_ = PurchaseOrderStatus::Sent;
    }
    void receiveItem(std::size_t index, double quantity) {
        if (status_ != PurchaseOrderStatus::Sent && status_ != PurchaseOrderStatus::PartiallyReceived)
            throw RestaurantException("Закупка не ожидает приёмки");
        if (index >= items_.size()) throw RestaurantException("Позиция закупки не найдена");
        items_[index].receive(quantity);
        status_ = allItemsReceived() ? PurchaseOrderStatus::Received : PurchaseOrderStatus::PartiallyReceived;
    }
    bool allItemsReceived() const {
        return !items_.empty() && std::all_of(items_.begin(), items_.end(), [](const PurchaseOrderItem& item) { return item.isComplete(); });
    }
    void cancel() {
        if (status_ == PurchaseOrderStatus::Received) throw RestaurantException("Полученную закупку нельзя отменить");
        status_ = PurchaseOrderStatus::Cancelled;
    }
    void addNote(const std::string& note) { note_ = note; }
    const Supplier& supplier() const { return supplier_; }
    const std::vector<PurchaseOrderItem>& items() const { return items_; }
};

class GoodsReceipt {
private:
    int receiptId_ = 0;
    int purchaseOrderId_ = 0;
    std::string receiptDate_;
    std::vector<StockMovement> movements_;
    bool confirmed_ = false;

public:
    GoodsReceipt() = default;
    GoodsReceipt(int id, const PurchaseOrder& order, std::string date)
        : receiptId_(id), purchaseOrderId_(order.id()), receiptDate_(std::move(date)) {
        if (id <= 0 || receiptDate_.empty()) throw RestaurantException("Некорректные данные приёмки");
    }
    void recordMovement(const StockMovement& movement) {
        if (confirmed_ || !movement.isIncoming()) throw RestaurantException("Нельзя добавить это движение в приёмку");
        movements_.push_back(movement);
    }
    void confirm() {
        if (movements_.empty()) throw RestaurantException("Пустую приёмку нельзя подтвердить");
        confirmed_ = true;
    }
    bool isConfirmed() const { return confirmed_; }
    const std::string& receiptDate() const { return receiptDate_; }
    double receivedQuantity() const {
        double total = 0.0;
        for (const auto& movement : movements_) total += movement.quantity();
        return total;
    }
    bool belongsToOrder(int orderId) const { return purchaseOrderId_ == orderId; }
};

class WasteRecord {
private:
    int wasteId_ = 0;
    int ingredientId_ = 0;
    double quantity_ = 0.0;
    std::string reason_;
    std::string date_;
    double unitCost_ = 0.0;

public:
    WasteRecord() = default;
    WasteRecord(int id, const Ingredient& ingredient, double quantity, std::string reason, std::string date)
        : wasteId_(id), ingredientId_(ingredient.id()), quantity_(quantity), reason_(std::move(reason)),
          date_(std::move(date)), unitCost_(ingredient.purchasePrice()) {
        if (id <= 0 || quantity <= 0.0 || reason_.empty() || date_.empty()) throw RestaurantException("Некорректная запись списания");
    }
    double calculateLoss() const { return quantity_ * unitCost_; }
    bool isSpoilage() const { return reason_ == "spoiled" || reason_ == "expired"; }
    bool concerns(const Ingredient& ingredient) const { return ingredient.id() == ingredientId_; }
    const std::string& date() const { return date_; }
    void changeReason(const std::string& reason) { if (!reason.empty()) reason_ = reason; }
};

} // namespace restaurant