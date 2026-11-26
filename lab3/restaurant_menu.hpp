#pragma once

#include "restaurant_exceptions.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <string>
#include <vector>
#include <utility>

namespace restaurant {

class MenuItem {
private:
    int itemId_ = 0;
    std::string name_;
    std::string description_;
    double price_ = 0.0;
    int preparationTimeMinutes_ = 0;
    bool available_ = true;

public:
    MenuItem() = default;
    MenuItem(int id, std::string name, std::string description, double price,
             int preparationTimeMinutes)
        : itemId_(id), name_(std::move(name)), description_(std::move(description)),
          price_(price), preparationTimeMinutes_(preparationTimeMinutes) {
        if (id <= 0 || name_.empty() || price < 0.0 || preparationTimeMinutes < 0) {
            throw InvalidOrderException("Некорректные данные позиции меню");
        }
    }

    int id() const { return itemId_; }
    const std::string& name() const { return name_; }
    const std::string& description() const { return description_; }
    double price() const { return price_; }
    int preparationTime() const { return preparationTimeMinutes_; }
    bool available() const { return available_; }

    void changePrice(double newPrice) {
        if (newPrice < 0.0) throw RestaurantException("Цена не может быть отрицательной");
        price_ = newPrice;
    }
    void markAvailable() { available_ = true; }
    void markUnavailable() { available_ = false; }
    bool matchesSearch(const std::string& query) const {
        std::string text = name_ + " " + description_;
        std::string needle = query;
        std::transform(text.begin(), text.end(), text.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        std::transform(needle.begin(), needle.end(), needle.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return text.find(needle) != std::string::npos;
    }
    bool canBeOrdered() const { return available_ && price_ >= 0.0; }
};

class Ingredient {
private:
    int ingredientId_ = 0;
    std::string name_;
    std::string unit_;
    double purchasePricePerUnit_ = 0.0;
    double caloriesPerUnit_ = 0.0;
    std::vector<std::string> allergens_;
    int expiryDays_ = 0;

public:
    Ingredient() = default;
    Ingredient(int id, std::string name, std::string unit, double price,
               double calories, int expiryDays)
        : ingredientId_(id), name_(std::move(name)), unit_(std::move(unit)),
          purchasePricePerUnit_(price), caloriesPerUnit_(calories), expiryDays_(expiryDays) {
        if (id <= 0 || name_.empty() || unit_.empty() || price < 0.0 || calories < 0.0 || expiryDays < 0) {
            throw RestaurantException("Некорректные данные ингредиента");
        }
    }

    int id() const { return ingredientId_; }
    const std::string& name() const { return name_; }
    const std::string& unit() const { return unit_; }
    double purchasePrice() const { return purchasePricePerUnit_; }
    double caloriesPerUnit() const { return caloriesPerUnit_; }
    const std::vector<std::string>& allergens() const { return allergens_; }

    double calculateCost(double quantity) const {
        if (quantity < 0.0) throw RestaurantException("Количество не может быть отрицательным");
        return purchasePricePerUnit_ * quantity;
    }
    double calculateCalories(double quantity) const {
        if (quantity < 0.0) throw RestaurantException("Количество не может быть отрицательным");
        return caloriesPerUnit_ * quantity;
    }
    bool containsAllergen(const std::string& allergen) const {
        return std::find(allergens_.begin(), allergens_.end(), allergen) != allergens_.end();
    }
    void addAllergen(const std::string& allergen) {
        if (!allergen.empty() && !containsAllergen(allergen)) allergens_.push_back(allergen);
    }
    void removeAllergen(const std::string& allergen) {
        allergens_.erase(std::remove(allergens_.begin(), allergens_.end(), allergen), allergens_.end());
    }
    bool expiresWithin(int days) const { return days >= 0 && expiryDays_ <= days; }
    bool isSuitableForDiet(const std::string& diet) const {
        if (diet == "vegan") {
            return !containsAllergen("meat") && !containsAllergen("fish") &&
                   !containsAllergen("milk") && !containsAllergen("egg");
        }
        if (diet == "vegetarian") return !containsAllergen("meat") && !containsAllergen("fish");
        if (diet == "gluten-free") return !containsAllergen("gluten");
        return true;
    }
};

class RecipeIngredient {
private:
    int ingredientId_ = 0;
    double quantity_ = 0.0;
    std::string unit_;
    bool optional_ = false;
    std::string preparationNote_;

public:
    RecipeIngredient() = default;
    RecipeIngredient(int ingredientId, double quantity, std::string unit,
                     bool optional = false, std::string note = {})
        : ingredientId_(ingredientId), quantity_(quantity), unit_(std::move(unit)),
          optional_(optional), preparationNote_(std::move(note)) {
        if (ingredientId <= 0 || quantity < 0.0 || unit_.empty()) {
            throw RestaurantException("Некорректная строка рецепта");
        }
    }

    int ingredientId() const { return ingredientId_; }
    double quantity() const { return quantity_; }
    const std::string& unit() const { return unit_; }
    bool optional() const { return optional_; }
    const std::string& preparationNote() const { return preparationNote_; }

    double calculateRequiredQuantity(int basePortions, int requestedPortions) const {
        if (basePortions <= 0 || requestedPortions < 0) {
            throw RestaurantException("Некорректное количество порций");
        }
        return quantity_ * requestedPortions / basePortions;
    }
    double calculateCost(const Ingredient& ingredient) const {
        if (ingredient.id() != ingredientId_) throw IngredientUnavailableException("Ингредиент не совпадает с рецептом");
        return ingredient.calculateCost(quantity_);
    }
    bool matchesIngredient(const Ingredient& ingredient) const {
        return ingredient.id() == ingredientId_ && ingredient.unit() == unit_;
    }
    double calculateQuantityWithWaste(double wastePercent) const {
        if (wastePercent < 0.0 || wastePercent >= 100.0) throw RestaurantException("Некорректный процент потерь");
        return quantity_ * 100.0 / (100.0 - wastePercent);
    }
};

class Recipe {
private:
    int recipeId_ = 0;
    std::string name_;
    std::string instructions_;
    int basePortions_ = 1;
    int preparationTimeMinutes_ = 0;
    std::vector<RecipeIngredient> ingredients_;

public:
    Recipe() = default;
    Recipe(int id, std::string name, std::string instructions, int portions, int minutes)
        : recipeId_(id), name_(std::move(name)), instructions_(std::move(instructions)),
          basePortions_(portions), preparationTimeMinutes_(minutes) {
        if (id <= 0 || name_.empty() || portions <= 0 || minutes < 0) {
            throw RestaurantException("Некорректные данные рецепта");
        }
    }

    int id() const { return recipeId_; }
    const std::string& name() const { return name_; }
    const std::string& instructions() const { return instructions_; }
    int preparationTime() const { return preparationTimeMinutes_; }
    const std::vector<RecipeIngredient>& ingredients() const { return ingredients_; }

    void addIngredient(const RecipeIngredient& ingredient) {
        if (containsIngredient(ingredient.ingredientId())) throw RestaurantException("Ингредиент уже есть в рецепте");
        ingredients_.push_back(ingredient);
    }
    void removeIngredient(int ingredientId) {
        auto it = std::remove_if(ingredients_.begin(), ingredients_.end(),
                                 [ingredientId](const RecipeIngredient& item) { return item.ingredientId() == ingredientId; });
        if (it == ingredients_.end()) throw IngredientUnavailableException("Ингредиент не найден в рецепте");
        ingredients_.erase(it, ingredients_.end());
    }
    bool containsIngredient(int ingredientId) const {
        return std::any_of(ingredients_.begin(), ingredients_.end(),
                           [ingredientId](const RecipeIngredient& item) { return item.ingredientId() == ingredientId; });
    }
    double calculateTotalCost(const std::vector<Ingredient>& catalog) const {
        double total = 0.0;
        for (const auto& row : ingredients_) {
            auto it = std::find_if(catalog.begin(), catalog.end(), [&row](const Ingredient& item) { return item.id() == row.ingredientId(); });
            if (it == catalog.end()) throw IngredientUnavailableException("Не найден ингредиент рецепта");
            total += row.calculateCost(*it);
        }
        return total;
    }
    double calculateCostPerPortion(const std::vector<Ingredient>& catalog) const {
        return calculateTotalCost(catalog) / basePortions_;
    }
    std::vector<RecipeIngredient> scaleIngredients(int requestedPortions) const {
        if (requestedPortions < 0) throw RestaurantException("Количество порций не может быть отрицательным");
        std::vector<RecipeIngredient> result;
        for (const auto& row : ingredients_) {
            result.emplace_back(row.ingredientId(), row.calculateRequiredQuantity(basePortions_, requestedPortions), row.unit(), row.optional());
        }
        return result;
    }
    bool containsAllergen(const std::string& allergen, const std::vector<Ingredient>& catalog) const {
        for (const auto& row : ingredients_) {
            auto it = std::find_if(catalog.begin(), catalog.end(), [&row](const Ingredient& item) { return item.id() == row.ingredientId(); });
            if (it != catalog.end() && it->containsAllergen(allergen)) return true;
        }
        return false;
    }
    bool isComplete() const { return !name_.empty() && basePortions_ > 0 && !ingredients_.empty(); }
};

class Dish {
private:
    MenuItem menuItem_;
    Recipe recipe_;
    std::string category_;
    double caloriesPerPortion_ = 0.0;
    bool vegetarian_ = false;
    bool spicy_ = false;

public:
    Dish() = default;
    Dish(MenuItem item, Recipe recipe, std::string category, double calories,
         bool vegetarian, bool spicy)
        : menuItem_(std::move(item)), recipe_(std::move(recipe)), category_(std::move(category)),
          caloriesPerPortion_(calories), vegetarian_(vegetarian), spicy_(spicy) {
        if (category_.empty() || calories < 0.0) throw RestaurantException("Некорректные данные блюда");
    }

    int id() const { return menuItem_.id(); }
    const MenuItem& menuItem() const { return menuItem_; }
    const Recipe& recipe() const { return recipe_; }
    const std::string& category() const { return category_; }
    double caloriesPerPortion() const { return caloriesPerPortion_; }

    double calculateCost(const std::vector<Ingredient>& catalog) const { return recipe_.calculateCostPerPortion(catalog); }
    bool isSuitableForDiet(const std::string& diet, const std::vector<Ingredient>& catalog) const {
        if (diet == "vegan") return vegetarian_ && !recipe_.containsAllergen("milk", catalog) && !recipe_.containsAllergen("egg", catalog);
        if (diet == "vegetarian") return vegetarian_;
        if (diet == "gluten-free") return !recipe_.containsAllergen("gluten", catalog);
        return true;
    }
    bool containsAllergen(const std::string& allergen, const std::vector<Ingredient>& catalog) const {
        return recipe_.containsAllergen(allergen, catalog);
    }
    void updateRecipe(const Recipe& newRecipe) {
        if (!newRecipe.isComplete()) throw RestaurantException("Нельзя назначить незаполненный рецепт");
        recipe_ = newRecipe;
    }
    double calculateProfit(const std::vector<Ingredient>& catalog) const {
        return menuItem_.price() - calculateCost(catalog);
    }
    bool isSpicy() const { return spicy_; }
    bool isVegetarian() const { return vegetarian_; }
};

class Drink {
private:
    MenuItem menuItem_;
    std::string drinkType_;
    int volumeMl_ = 0;
    bool alcoholic_ = false;
    double alcoholPercentage_ = 0.0;

public:
    Drink() = default;
    Drink(MenuItem item, std::string type, int volume, bool alcoholic, double alcoholPercentage)
        : menuItem_(std::move(item)), drinkType_(std::move(type)), volumeMl_(volume),
          alcoholic_(alcoholic), alcoholPercentage_(alcoholPercentage) {
        if (drinkType_.empty() || volume <= 0 || alcoholPercentage < 0.0 || alcoholPercentage > 100.0 || (!alcoholic && alcoholPercentage != 0.0)) {
            throw RestaurantException("Некорректные данные напитка");
        }
    }

    int id() const { return menuItem_.id(); }
    const MenuItem& menuItem() const { return menuItem_; }
    const std::string& drinkType() const { return drinkType_; }
    int volumeMl() const { return volumeMl_; }
    bool isAlcoholFree() const { return !alcoholic_ || alcoholPercentage_ == 0.0; }
    double calculatePureAlcoholVolume() const { return volumeMl_ * alcoholPercentage_ / 100.0; }
    bool isSuitableForChildren() const { return isAlcoholFree(); }
    void changeVolume(int newVolume) {
        if (newVolume <= 0) throw RestaurantException("Объём должен быть положительным");
        volumeMl_ = newVolume;
    }
};

class MenuSection {
private:
    int sectionId_ = 0;
    std::string title_;
    std::string description_;
    int displayOrder_ = 0;
    bool visible_ = true;
    std::vector<Dish> dishes_;
    std::vector<Drink> drinks_;

public:
    MenuSection() = default;
    MenuSection(int id, std::string title, std::string description, int order)
        : sectionId_(id), title_(std::move(title)), description_(std::move(description)), displayOrder_(order) {
        if (id <= 0 || title_.empty() || order < 0) throw RestaurantException("Некорректный раздел меню");
    }

    int id() const { return sectionId_; }
    const std::string& title() const { return title_; }
    const std::string& description() const { return description_; }
    int displayOrder() const { return displayOrder_; }
    const std::vector<Dish>& dishes() const { return dishes_; }
    const std::vector<Drink>& drinks() const { return drinks_; }

    void addDish(const Dish& dish) {
        if (findDishById(dish.id()) != nullptr || findDrinkById(dish.id()) != nullptr) throw RestaurantException("Позиция уже существует в разделе");
        dishes_.push_back(dish);
    }
    void addDrink(const Drink& drink) {
        if (findDishById(drink.id()) != nullptr || findDrinkById(drink.id()) != nullptr) throw RestaurantException("Позиция уже существует в разделе");
        drinks_.push_back(drink);
    }
    void removeItem(int itemId) {
        const auto dishIt = std::remove_if(dishes_.begin(), dishes_.end(), [itemId](const Dish& item) { return item.id() == itemId; });
        if (dishIt != dishes_.end()) { dishes_.erase(dishIt, dishes_.end()); return; }
        const auto drinkIt = std::remove_if(drinks_.begin(), drinks_.end(), [itemId](const Drink& item) { return item.id() == itemId; });
        if (drinkIt == drinks_.end()) throw DishNotFoundException("Позиция меню не найдена");
        drinks_.erase(drinkIt, drinks_.end());
    }
    const Dish* findDishById(int itemId) const {
        auto it = std::find_if(dishes_.begin(), dishes_.end(), [itemId](const Dish& item) { return item.id() == itemId; });
        return it == dishes_.end() ? nullptr : &*it;
    }
    const Drink* findDrinkById(int itemId) const {
        auto it = std::find_if(drinks_.begin(), drinks_.end(), [itemId](const Drink& item) { return item.id() == itemId; });
        return it == drinks_.end() ? nullptr : &*it;
    }
    int countAvailableItems() const {
        int count = 0;
        for (const auto& item : dishes_) if (item.menuItem().canBeOrdered()) ++count;
        for (const auto& item : drinks_) if (item.menuItem().canBeOrdered()) ++count;
        return count;
    }
    void sortItemsByPrice() {
        std::sort(dishes_.begin(), dishes_.end(), [](const Dish& a, const Dish& b) { return a.menuItem().price() < b.menuItem().price(); });
        std::sort(drinks_.begin(), drinks_.end(), [](const Drink& a, const Drink& b) { return a.menuItem().price() < b.menuItem().price(); });
    }
    void setVisible(bool value) { visible_ = value; }
    bool visible() const { return visible_; }
};

enum class MenuStatus { Draft, Active, Archived };

class Menu {
private:
    int menuId_ = 0;
    std::string name_;
    std::string description_;
    MenuStatus status_ = MenuStatus::Draft;
    std::vector<MenuSection> sections_;
    std::string validFrom_;
    std::string validUntil_;

public:
    Menu() = default;
    Menu(int id, std::string name, std::string description, std::string from, std::string until)
        : menuId_(id), name_(std::move(name)), description_(std::move(description)),
          validFrom_(std::move(from)), validUntil_(std::move(until)) {
        if (id <= 0 || name_.empty()) throw RestaurantException("Некорректные данные меню");
    }

    int id() const { return menuId_; }
    const std::string& name() const { return name_; }
    const std::string& description() const { return description_; }
    const std::string& validFrom() const { return validFrom_; }
    const std::string& validUntil() const { return validUntil_; }
    MenuStatus status() const { return status_; }
    const std::vector<MenuSection>& sections() const { return sections_; }

    void addSection(const MenuSection& section) {
        if (findSection(section.id()) != nullptr) throw RestaurantException("Раздел уже существует");
        sections_.push_back(section);
    }
    void removeSection(int sectionId) {
        auto it = std::remove_if(sections_.begin(), sections_.end(), [sectionId](const MenuSection& section) { return section.id() == sectionId; });
        if (it == sections_.end()) throw RestaurantException("Раздел меню не найден");
        sections_.erase(it, sections_.end());
    }
    MenuSection* findSection(int sectionId) {
        auto it = std::find_if(sections_.begin(), sections_.end(), [sectionId](const MenuSection& section) { return section.id() == sectionId; });
        return it == sections_.end() ? nullptr : &*it;
    }
    const MenuSection* findSection(int sectionId) const {
        auto it = std::find_if(sections_.begin(), sections_.end(), [sectionId](const MenuSection& section) { return section.id() == sectionId; });
        return it == sections_.end() ? nullptr : &*it;
    }
    void activate() {
        if (sections_.empty()) throw RestaurantException("Нельзя активировать пустое меню");
        status_ = MenuStatus::Active;
    }
    void archive() { status_ = MenuStatus::Archived; }
    bool isAvailableOn(const std::string& date) const {
        if (status_ != MenuStatus::Active || date.empty()) return false;
        return (validFrom_.empty() || date >= validFrom_) && (validUntil_.empty() || date <= validUntil_);
    }
    const Dish* findDishById(int itemId) const {
        for (const auto& section : sections_) if (const Dish* dish = section.findDishById(itemId)) return dish;
        return nullptr;
    }
    const Drink* findDrinkById(int itemId) const {
        for (const auto& section : sections_) if (const Drink* drink = section.findDrinkById(itemId)) return drink;
        return nullptr;
    }
    std::vector<int> searchItems(const std::string& query) const {
        std::vector<int> result;
        for (const auto& section : sections_) {
            for (const auto& dish : section.dishes()) if (dish.menuItem().matchesSearch(query)) result.push_back(dish.id());
            for (const auto& drink : section.drinks()) if (drink.menuItem().matchesSearch(query)) result.push_back(drink.id());
        }
        return result;
    }
    int countAvailableItems() const {
        int count = 0;
        for (const auto& section : sections_) if (section.visible()) count += section.countAvailableItems();
        return count;
    }
};

} // namespace restaurant