#include "restaurant_management.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

using namespace restaurant;

namespace {

int readInt(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    try { return std::stoi(value); }
    catch (...) { return -1; }
}

double readDouble(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    try { return std::stod(value); }
    catch (...) { return -1.0; }
}

std::string readText(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

Menu createDemoMenu(std::vector<Ingredient>& ingredients) {
    Ingredient flour(1, "Мука", "г", 0.04, 3.64, 180);
    flour.addAllergen("gluten");
    Ingredient cheese(2, "Сыр", "г", 0.18, 4.02, 30);
    cheese.addAllergen("milk");
    ingredients = {flour, cheese};

    Recipe recipe(1, "Пицца с сыром", "Раскатать тесто, добавить сыр и запечь", 1, 15);
    recipe.addIngredient(RecipeIngredient(1, 120.0, "г"));
    recipe.addIngredient(RecipeIngredient(2, 80.0, "г"));
    MenuItem item(101, "Пицца Маргарита", "Пицца с томатами и сыром", 12.50, 15);
    Dish dish(item, recipe, "Основные блюда", 650.0, true, false);

    MenuSection section(1, "Основные блюда", "Горячие блюда", 1);
    section.addDish(dish);
    section.addDrink(Drink(MenuItem(201, "Лимонад", "Домашний лимонад", 3.50, 2), "Безалкогольный", 300, false, 0.0));

    Menu menu(1, "Основное меню", "Меню ресторана", "2026-01-01", "2027-12-31");
    menu.addSection(section);
    menu.activate();
    return menu;
}

Restaurant createDemoRestaurant(const Menu& menu) {
    Restaurant restaurant(1, "Ресторан Вкус", "+375-17-000-00-00",
                           RestaurantAddress("Ленина", "15", "Минск", "220030", "Беларусь"));
    restaurant.updateOpeningHours(OpeningHours("Понедельник", 600, 1320));
    restaurant.updateOpeningHours(OpeningHours("Вторник", 600, 1320));

    TableZone mainZone(1, "Основной зал", 1, false, false);
    mainZone.addFeature("Обычная посадка");
    DiningHall hall(1, "Главный зал", 1, 40);
    hall.addZone(mainZone);
    hall.addTable(RestaurantTable(1, 1, 2, true, 1));
    hall.addTable(RestaurantTable(2, 2, 4, false, 1));
    hall.addTable(RestaurantTable(3, 3, 6, false, 1));
    restaurant.addHall(hall);
    restaurant.setActiveMenu(menu);
    return restaurant;
}

void printMenu(const Menu& menu) {
    std::cout << "\n--- " << menu.name() << " ---\n";
    for (const auto& section : menu.sections()) {
        if (!section.visible()) continue;
        std::cout << section.title() << ":\n";
        for (const auto& dish : section.dishes()) {
            std::cout << "  " << dish.id() << ". " << dish.menuItem().name()
                      << " — " << dish.menuItem().price() << "\n";
        }
        for (const auto& drink : section.drinks()) {
            std::cout << "  " << drink.id() << ". " << drink.menuItem().name()
                      << " — " << drink.menuItem().price() << "\n";
        }
    }
}

void registerCustomer(std::vector<Customer>& customers) {
    std::string name = readText("Имя клиента: ");
    std::string phone = readText("Телефон: ");
    std::string email = readText("Email (можно оставить пустым): ");
    int id = static_cast<int>(customers.size()) + 1;
    CustomerProfile profile(phone, email, "");
    customers.emplace_back(id, name, profile, LoyaltyCard("CARD-" + std::to_string(id)), "2026-11-26");
    std::cout << "Клиент зарегистрирован. ID: " << id << "\n";
}

void bookTable(Restaurant& restaurant, const std::vector<Customer>& customers,
               std::vector<Reservation>& reservations) {
    if (customers.empty()) { std::cout << "Сначала зарегистрируйте клиента.\n"; return; }
    const int customerId = readInt("ID клиента: ");
    auto customer = std::find_if(customers.begin(), customers.end(), [customerId](const Customer& item) { return item.id() == customerId; });
    if (customer == customers.end()) { std::cout << "Клиент не найден.\n"; return; }
    const int tableNumber = readInt("Номер столика: ");
    const int guests = readInt("Количество гостей: ");
    RestaurantTable* table = restaurant.findTable(tableNumber);
    if (table == nullptr) { std::cout << "Столик не найден.\n"; return; }
    if (!table->canAccommodate(guests)) { std::cout << "Столик недоступен или недостаточно мест.\n"; return; }

    const int id = static_cast<int>(reservations.size()) + 1;
    try {
        Reservation reservation(id, *customer, table->id(), guests, "2026-11-26", 720, 90);
        for (const auto& existing : reservations) {
            if (reservation.overlaps(existing)) throw InvalidReservationException("На это время столик уже забронирован");
        }
        reservation.confirm(*table);
        reservations.push_back(reservation);
        std::cout << "Бронирование подтверждено. Номер: " << id << "\n";
    } catch (const RestaurantException& error) {
        std::cout << "Не удалось оформить бронирование: " << error.what() << "\n";
    }
}

void createOrder(const Menu& menu, const std::vector<Customer>& customers,
                 Restaurant& restaurant, std::vector<Reservation>& reservations,
                 std::vector<Order>& orders) {
    if (customers.empty()) { std::cout << "Сначала зарегистрируйте клиента.\n"; return; }

    const int customerId = readInt("ID клиента: ");
    auto customer = std::find_if(customers.begin(), customers.end(),
        [customerId](const Customer& item) { return item.id() == customerId; });
    if (customer == customers.end()) { std::cout << "Клиент не найден.\n"; return; }

    std::cout << "\nПодтверждённые бронирования клиента:\n";
    for (const auto& reservation : reservations) {
        if (reservation.isForCustomer(customerId) &&
            reservation.status() == ReservationStatus::Confirmed) {
            const RestaurantTable* table = restaurant.findTableById(reservation.tableId());
            const int tableNumber = table == nullptr ? 0 : table->number();
            std::cout << "  Бронирование " << reservation.id()
                      << ", столик " << tableNumber
                      << ", дата " << reservation.date() << "\n";
        }
    }

    const int reservationId = readInt("ID бронирования (0 — заказ на вынос): ");
    Reservation* selectedReservation = nullptr;
    RestaurantTable* selectedTable = nullptr;
    if (reservationId != 0) {
        auto reservation = std::find_if(reservations.begin(), reservations.end(),
            [reservationId](const Reservation& item) { return item.id() == reservationId; });
        if (reservation == reservations.end() || !reservation->isForCustomer(customerId) ||
            reservation->status() != ReservationStatus::Confirmed) {
            std::cout << "Подтверждённое бронирование клиента не найдено.\n";
            return;
        }
        selectedTable = restaurant.findTableById(reservation->tableId());
        if (selectedTable == nullptr || selectedTable->status() != TableStatus::Reserved) {
            std::cout << "Забронированный столик сейчас недоступен.\n";
            return;
        }
        selectedReservation = &*reservation;
    }

    printMenu(menu);
    const int itemId = readInt("ID блюда или напитка: ");
    MenuItem selectedItem;
    const Dish* dish = menu.findDishById(itemId);
    const Drink* drink = menu.findDrinkById(itemId);
    if (dish != nullptr) selectedItem = dish->menuItem();
    else if (drink != nullptr) selectedItem = drink->menuItem();
    else { std::cout << "Позиция меню не найдена.\n"; return; }

    const int quantity = readInt("Количество: ");
    try {
        const int tableId = selectedReservation == nullptr ? 0 : selectedReservation->tableId();
        Order order(static_cast<int>(orders.size()) + 1, *customer, tableId, "2026-11-26 12:00");
        order.addItem(selectedItem, quantity);
        if (selectedReservation != nullptr) order.attachReservation(*selectedReservation);
        order.changeStatus(OrderStatus::Accepted, "2026-11-26 12:01");

        if (selectedReservation != nullptr) selectedReservation->markSeated(*selectedTable);

        orders.push_back(order);
        std::cout << "Заказ создан. Номер: " << order.id()
                  << ", сумма: " << order.calculateTotal();
        if (selectedReservation != nullptr)
            std::cout << ", столик занят: " << selectedTable->number();
        else
            std::cout << ", заказ на вынос";
        std::cout << "\n";
    } catch (const RestaurantException& error) {
        std::cout << "Не удалось создать заказ: " << error.what() << "\n";
    }
}

void finishSuccessfulPayment(Order& order, Reservation* reservation,
                             RestaurantTable* table, Bill& bill,
                             RestaurantStatistics& statistics,
                             const Payment& payment, double change) {
    if (reservation != nullptr) reservation->complete(*table);
    order.changeStatus(OrderStatus::Paid, "2026-11-26 12:30");
    bill.close();
    statistics.recordPayment(payment);
    statistics.recordOrder(order);

    const std::string paymentName = payment.method() == "cash"
        ? "наличными" : "банковской картой";
    std::cout << "Оплата выполнена " << paymentName << ".\n";
    if (payment.method() == "cash")
        std::cout << "Сдача: " << change << "\n";

    if (reservation != nullptr) {
        std::cout << "Бронирование " << reservation->id()
                  << " завершено. Столик " << table->number()
                  << " ожидает уборки.\n";
    }
}

void payLastOrder(Restaurant& restaurant, std::vector<Reservation>& reservations,
                  std::vector<Order>& orders, RestaurantStatistics& statistics) {
    if (orders.empty()) { std::cout << "Заказов пока нет.\n"; return; }

    Order& order = orders.back();
    Reservation* reservation = nullptr;
    RestaurantTable* table = nullptr;
    try {
        if (order.status() == OrderStatus::Paid || order.status() == OrderStatus::Cancelled)
            throw InvalidOrderException("Заказ уже закрыт");

        if (order.reservationId() != 0) {
            auto found = std::find_if(reservations.begin(), reservations.end(),
                [&order](const Reservation& item) { return item.id() == order.reservationId(); });
            if (found == reservations.end() || !found->isForCustomer(order.customerId()) ||
                found->status() != ReservationStatus::Seated || found->tableId() != order.tableId())
                throw InvalidReservationException("Не удалось найти активное бронирование заказа");

            reservation = &*found;
            table = restaurant.findTableById(order.tableId());
            if (table == nullptr || table->status() != TableStatus::Occupied)
                throw TableUnavailableException("Столик заказа не находится в занятом состоянии");
        }

        Bill bill(static_cast<int>(orders.size()), order);
        const double total = bill.calculateTotal();
        std::cout << "К оплате: " << total << "\n"
                  << "Выберите способ оплаты:\n"
                  << "1. Наличными\n"
                  << "2. Банковской картой\n"
                  << "0. Отмена\n";
        const int paymentMethod = readInt("Ваш выбор: ");

        if (paymentMethod == 0) {
            std::cout << "Оплата отменена. Заказ и бронирование не изменены.\n";
            return;
        }

        if (paymentMethod == 1) {
            const double received = readDouble("Получено наличными: ");
            CashPayment payment(static_cast<int>(orders.size()), total, received);
            if (!payment.process()) throw PaymentFailedException("Оплата наличными не прошла");
            finishSuccessfulPayment(order, reservation, table, bill,
                                    statistics, payment, payment.change());
        } else if (paymentMethod == 2) {
            const std::string lastFourDigits = readText("Последние 4 цифры карты: ");
            const bool validDigits = lastFourDigits.size() == 4 &&
                std::all_of(lastFourDigits.begin(), lastFourDigits.end(),
                    [](unsigned char character) { return std::isdigit(character) != 0; });
            if (!validDigits)
                throw PaymentFailedException("Нужно указать ровно 4 цифры карты");

            CardPayment payment(static_cast<int>(orders.size()), total,
                                "****" + lastFourDigits);
            const int approval = readInt(
                "Подтвердить тестовую оплату? (1 — да, 0 — отказ): ");
            if (approval != 1) {
                std::cout << "Оплата картой отклонена. Заказ и бронирование не изменены.\n";
                return;
            }

            // Код имитирует ответ платёжного сервиса; реальный банк не вызывается.
            payment.approve("DEMO-AUTH");
            if (!payment.process()) throw PaymentFailedException("Оплата картой не прошла");
            finishSuccessfulPayment(order, reservation, table, bill,
                                    statistics, payment, 0.0);
        } else {
            std::cout << "Неизвестный способ оплаты. Заказ не изменён.\n";
        }
    } catch (const RestaurantException& error) {
        std::cout << "Ошибка оплаты: " << error.what() << "\n";
    }
}

void cleanTable(Restaurant& restaurant) {
    const int tableNumber = readInt("Номер столика после уборки: ");
    RestaurantTable* table = restaurant.findTable(tableNumber);
    if (table == nullptr) {
        std::cout << "Столик не найден.\n";
        return;
    }
    try {
        table->markCleaned();
        std::cout << "Столик " << tableNumber << " убран и снова доступен.\n";
    } catch (const RestaurantException& error) {
        std::cout << "Не удалось изменить состояние столика: " << error.what() << "\n";
    }
}

void showStatistics(const RestaurantStatistics& statistics) {
    std::cout << "\n--- Статистика ---\n"
              << "Всего заказов: " << statistics.totalOrders() << "\n"
              << "Завершено заказов: " << statistics.completedOrders() << "\n"
              << "Выручка после возвратов: " << statistics.netRevenue() << "\n"
              << "Средняя стоимость заказа: " << statistics.averageOrderValue() << "\n";
}

const char* tableStatusName(TableStatus status) {
    switch (status) {
        case TableStatus::Available: return "свободен";
        case TableStatus::Reserved: return "забронирован";
        case TableStatus::Occupied: return "занят";
        case TableStatus::NeedsCleaning: return "требует уборки";
        case TableStatus::OutOfService: return "не работает";
    }
    return "неизвестно";
}

void showTables(const Restaurant& restaurant) {
    std::cout << "\n--- Столики ресторана ---\n";
    for (const auto& hall : restaurant.halls()) {
        std::cout << hall.name() << ":\n";
        for (const auto& table : hall.tables()) {
            std::cout << "  Столик " << table.number() << " — "
                      << tableStatusName(table.status()) << ", мест: "
                      << table.capacity() << "\n";
        }
    }
    std::cout << "Общая вместимость: " << restaurant.calculateTotalCapacity() << "\n";
}

void printActions() {
    std::cout << "\n========== СИСТЕМА УПРАВЛЕНИЯ РЕСТОРАНОМ =========="
              << "\n1. Показать меню"
              << "\n2. Зарегистрировать клиента"
              << "\n3. Забронировать столик"
              << "\n4. Создать заказ"
              << "\n5. Оплатить последний заказ"
              << "\n6. Показать столики"
              << "\n7. Показать статистику"
              << "\n8. Отметить уборку столика"
              << "\n0. Выход\n";
}

} // namespace

int main() {
    std::vector<Ingredient> ingredients;
    Menu menu = createDemoMenu(ingredients);
    Restaurant restaurant = createDemoRestaurant(menu);
    std::vector<Customer> customers;
    std::vector<Reservation> reservations;
    std::vector<Order> orders;
    RestaurantStatistics statistics;

    std::cout << "Добро пожаловать в " << restaurant.name() << "!\n";
    int action = -1;
    while (action != 0) {
        printActions();
        action = readInt("Выберите действие: ");
        switch (action) {
            case 1: printMenu(menu); break;
            case 2: registerCustomer(customers); break;
            case 3: bookTable(restaurant, customers, reservations); break;
            case 4: createOrder(menu, customers, restaurant, reservations, orders); break;
            case 5: payLastOrder(restaurant, reservations, orders, statistics); break;
            case 6: showTables(restaurant); break;
            case 7: showStatistics(statistics); break;
            case 8: cleanTable(restaurant); break;
            case 0: std::cout << "Работа программы завершена.\n"; break;
            default: std::cout << "Неизвестная команда.\n"; break;
        }
    }
    return 0;
}