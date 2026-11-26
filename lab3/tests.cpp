#include "restaurant_management.hpp"

#include <cmath>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

using namespace restaurant;

namespace {

int checks = 0;

void check(bool condition, const std::string& message) {
    ++checks;
    if (!condition) throw std::runtime_error("Проверка не пройдена: " + message);
}

template <typename Exception, typename Function>
void expectThrow(Function function, const std::string& message) {
    ++checks;
    try {
        function();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error("Ожидалось исключение: " + message);
}

bool closeTo(double a, double b) { return std::abs(a - b) < 0.001; }

Menu makeMenu(std::vector<Ingredient>& ingredients) {
    Ingredient flour(1, "Мука", "г", 0.04, 3.6, 180);
    flour.addAllergen("gluten");
    Ingredient cheese(2, "Сыр", "г", 0.18, 4.0, 30);
    cheese.addAllergen("milk");
    ingredients = {flour, cheese};
    Recipe recipe(1, "Тесто с сыром", "Смешать и запечь", 1, 15);
    recipe.addIngredient(RecipeIngredient(1, 100.0, "г"));
    recipe.addIngredient(RecipeIngredient(2, 50.0, "г"));
    MenuItem pizzaItem(101, "Пицца", "Пицца с сыром", 10.0, 15);
    Dish pizza(pizzaItem, recipe, "Основное", 500.0, true, false);
    Drink lemonade(MenuItem(201, "Лимонад", "Домашний напиток", 3.0, 2), "soft", 300, false, 0.0);
    MenuSection section(1, "Блюда", "Основные блюда", 1);
    section.addDish(pizza);
    section.addDrink(lemonade);
    Menu menu(1, "Основное меню", "Тестовое меню", "2026-01-01", "2027-12-31");
    menu.addSection(section);
    menu.activate();
    return menu;
}

Customer makeCustomer(int id = 1) {
    CustomerProfile profile("+375291112233", "guest@example.com", "1999-01-01");
    return Customer(id, "Иван Иванов", profile, LoyaltyCard("CARD-" + std::to_string(id)), "2026-01-01");
}

void testCore() {
    RestaurantAddress address("Ленина", "10", "Минск", "220030", "Беларусь");
    check(address.isValid(), "адрес корректен");
    check(address.formatFullAddress().find("Минск") != std::string::npos, "форматирование адреса");
    check(address.isLocatedIn("Минск") && !address.isLocatedIn("Гродно"), "проверка города");
    check(!RestaurantAddress().isValid(), "пустой адрес некорректен");

    OpeningHours hours("Понедельник", 600, 1320);
    check(hours.isValid() && !hours.isClosed(), "график корректен");
    check(hours.isOpenAt(10, 0) && !hours.isOpenAt(22, 0), "проверка времени работы");
    check(!hours.isOpenAt(25, 0) && closeTo(hours.getDurationMinutes(), 720), "границы времени и длительность");
    hours.setSpecialNote("Праздничный график");
    check(hours.specialNote() == "Праздничный график", "заметка к графику");
    hours.setWorkingPeriod(540, 1260);
    check(hours.isOpenAt(9, 0), "изменение графика");
    hours.closeForDay();
    check(hours.isClosed() && !hours.isOpenAt(10, 0), "выходной день");
    expectThrow<RestaurantException>([] { OpeningHours("Пятница", 900, 800); }, "неверный график");
    expectThrow<RestaurantException>([&] { hours.setWorkingPeriod(900, 800); }, "неверный интервал");

    TableZone zone(1, "Терраса", 1, true, true);
    check(zone.matchesPreferences(true, true) && !zone.matchesPreferences(false, false) == false, "предпочтения зоны");
    check(zone.floor() == 1, "этаж зоны");
    zone.addFeature("вид на улицу");
    zone.addFeature("вид на улицу");
    check(zone.hasFeature("вид на улицу"), "добавление особенности");
    zone.removeFeature("вид на улицу");
    check(!zone.hasFeature("вид на улицу"), "удаление особенности");

    RestaurantTable table(1, 1, 4, true, 1);
    check(table.canAccommodate(3) && !table.canAccommodate(5) && table.isNearWindow(), "вместимость и расположение столика");
    table.reserve();
    check(table.status() == TableStatus::Reserved, "резервирование столика");
    table.occupy();
    table.markNeedsCleaning();
    check(table.status() == TableStatus::NeedsCleaning, "столик ожидает уборки");
    table.markCleaned();
    check(table.status() == TableStatus::Available, "столик снова доступен после уборки");
    expectThrow<TableUnavailableException>([&] { table.markCleaned(); }, "повторная уборка свободного столика");
    table.reserve();
    table.release();
    check(table.status() == TableStatus::Available, "освобождение забронированного столика");
    table.reserve();
    table.occupy();
    expectThrow<TableUnavailableException>([&] { table.release(); }, "нельзя пропустить уборку занятого столика");
    table.markNeedsCleaning();
    table.markCleaned();
    table.takeOutOfService();
    expectThrow<TableUnavailableException>([&] { table.release(); }, "освобождение неисправного столика");
    table.restoreService();
    check(table.status() == TableStatus::NeedsCleaning && table.matchesPreferences(true), "возврат столика в работу");
    expectThrow<RestaurantException>([] { RestaurantTable bad(0, 1, 2, false, 1); }, "неверный столик");

    DiningHall hall(1, "Главный зал", 1, 20);
    hall.addZone(zone);
    hall.addTable(RestaurantTable(2, 2, 2, false, 1));
    hall.addTable(RestaurantTable(3, 3, 4, true, 1));
    check(hall.floor() == 1 && hall.capacity() == 20, "параметры зала");
    check(hall.findTable(2) != nullptr && hall.findTable(9) == nullptr, "поиск столика в зале");
    check(hall.findAvailableTables(3).size() == 1 && hall.calculateFreeSeats() == 6, "свободные столики и места");
    hall.assignTableToZone(2, 1);
    hall.close();
    check(!hall.isOpen(), "закрытие зала");
    hall.open();
    check(hall.isOpen(), "открытие зала");
    hall.removeTable(2);
    expectThrow<TableNotFoundException>([&] { hall.removeTable(2); }, "удаление отсутствующего столика");
    expectThrow<RestaurantException>([&] { hall.addZone(zone); }, "добавление повторной зоны");
    expectThrow<RestaurantException>([] { DiningHall small(9, "Малый", 1, 2); small.addTable(RestaurantTable(9, 9, 4, false, 1)); }, "превышение вместимости зала");

    std::vector<Ingredient> catalog;
    Menu menu = makeMenu(catalog);
    Restaurant restaurant(1, "Тестовый ресторан", "123", address);
    restaurant.updateOpeningHours(OpeningHours("Понедельник", 600, 1320));
    check(restaurant.phoneNumber() == "123" && restaurant.address().city() == "Минск", "контактные данные ресторана");
    restaurant.addHall(hall);
    check(restaurant.isOpenAt("Понедельник", 12, 0), "проверка графика ресторана");
    restaurant.setActiveMenu(menu);
    check(restaurant.activeMenu().id() == menu.id(), "установка активного меню");
    check(restaurant.calculateTotalCapacity() == 4, "общая вместимость");
    check(restaurant.findAvailableTables(2).size() == 1, "поиск столиков во всех залах");
    restaurant.addHall(DiningHall(2, "Малый зал", 1, 10));
    restaurant.removeHall(2);
    expectThrow<RestaurantException>([&] { restaurant.setActiveMenu(Menu(2, "Черновик", "", "", "")); }, "назначение чернового меню");
}

void testMenu() {
    MenuItem item(10, "Суп", "Овощной суп", 5.0, 10);
    check(item.canBeOrdered() && item.matchesSearch("суп"), "поиск позиции меню");
    item.changePrice(6.0);
    check(closeTo(item.price(), 6.0), "изменение цены");
    item.markUnavailable();
    check(!item.canBeOrdered(), "недоступная позиция");
    item.markAvailable();
    expectThrow<RestaurantException>([&] { item.changePrice(-1.0); }, "отрицательная цена");

    Ingredient apple(1, "Яблоко", "г", 0.5, 50.0, 20);
    apple.addAllergen("pollen");
    check(closeTo(apple.calculateCost(2), 1.0) && closeTo(apple.calculateCalories(2), 100.0), "расчёт ингредиента");
    check(apple.containsAllergen("pollen") && !apple.isSuitableForDiet("vegan") == false, "аллергены и рацион");
    check(apple.expiresWithin(20), "срок хранения");
    apple.removeAllergen("pollen");
    check(!apple.containsAllergen("pollen"), "удаление аллергена");
    expectThrow<RestaurantException>([&] { apple.calculateCost(-2); }, "отрицательное количество ингредиента");

    RecipeIngredient row(1, 100.0, "г");
    check(closeTo(row.calculateRequiredQuantity(2, 4), 200.0), "пересчёт ингредиента на порции");
    check(closeTo(row.calculateCost(apple), 50.0), "проверка связи ингредиента и рецепта");
    check(row.matchesIngredient(apple) && row.preparationNote().empty(), "сопоставление ингредиента");
    check(closeTo(row.calculateQuantityWithWaste(10), 111.111), "учёт потерь");
    expectThrow<RestaurantException>([&] { row.calculateQuantityWithWaste(100); }, "некорректные потери");

    Ingredient flour(2, "Мука", "г", 0.02, 3.0, 100);
    flour.addAllergen("gluten");
    Recipe recipe(1, "Тесто", "Смешать", 2, 20);
    recipe.addIngredient(RecipeIngredient(2, 200, "г"));
    check(recipe.containsIngredient(2) && recipe.isComplete(), "состав рецепта");
    check(closeTo(recipe.calculateTotalCost({flour}), 4.0) && closeTo(recipe.calculateCostPerPortion({flour}), 2.0), "себестоимость рецепта");
    check(recipe.scaleIngredients(4).size() == 1 && closeTo(recipe.scaleIngredients(4)[0].quantity(), 400.0), "масштабирование рецепта");
    check(recipe.containsAllergen("gluten", {flour}), "поиск аллергена в рецепте");
    recipe.removeIngredient(2);
    check(!recipe.containsIngredient(2) && !recipe.isComplete(), "удаление ингредиента рецепта");
    expectThrow<IngredientUnavailableException>([&] { recipe.removeIngredient(999); }, "удаление отсутствующего ингредиента");
    expectThrow<IngredientUnavailableException>([&] { row.calculateCost(flour); }, "несовпадающий ингредиент");

    Recipe pizzaRecipe(2, "Пицца", "Запечь", 1, 15);
    pizzaRecipe.addIngredient(RecipeIngredient(2, 100, "г"));
    MenuItem pizzaItem(100, "Пицца", "Сырная пицца", 10.0, 15);
    Dish dish(pizzaItem, pizzaRecipe, "Основное", 500, true, true);
    check(dish.isSpicy() && dish.isVegetarian(), "свойства блюда");
    check(closeTo(dish.calculateCost({flour}), 2.0) && closeTo(dish.calculateProfit({flour}), 8.0), "себестоимость и прибыль блюда");
    check(dish.containsAllergen("gluten", {flour}), "аллергены блюда");
    check(dish.isSuitableForDiet("vegan", {flour}) && dish.isSuitableForDiet("vegetarian", {flour}), "проверка диеты");
    pizzaRecipe.addIngredient(RecipeIngredient(3, 50, "г"));
    dish.updateRecipe(pizzaRecipe);
    expectThrow<RestaurantException>([&] { dish.updateRecipe(recipe); }, "замена пустым рецептом");

    Drink drink(MenuItem(200, "Вода", "Минеральная вода", 2.0, 1), "water", 500, false, 0.0);
    check(drink.isAlcoholFree() && drink.isSuitableForChildren(), "безалкогольный напиток");
    drink.changeVolume(600);
    check(closeTo(drink.calculatePureAlcoholVolume(), 0.0), "объём чистого алкоголя");
    expectThrow<RestaurantException>([&] { drink.changeVolume(0); }, "нулевой объём напитка");

    MenuSection section(1, "Основное", "Блюда", 1);
    section.addDish(dish);
    section.addDrink(drink);
    check(section.findDishById(100) != nullptr && section.findDrinkById(200) != nullptr, "поиск в разделе");
    check(section.countAvailableItems() == 2, "количество доступных позиций");
    section.sortItemsByPrice();
    section.setVisible(false);
    check(!section.visible(), "видимость раздела");
    expectThrow<RestaurantException>([&] { section.addDish(dish); }, "добавление повторного блюда");
    section.removeItem(200);
    expectThrow<DishNotFoundException>([&] { section.removeItem(999); }, "удаление отсутствующей позиции");

    Menu menu(1, "Основное меню", "Описание", "2026-01-01", "2027-01-01");
    expectThrow<RestaurantException>([&] { menu.activate(); }, "активация пустого меню");
    menu.addSection(section);
    check(menu.findSection(1) != nullptr && menu.findSection(9) == nullptr, "поиск раздела меню");
    menu.activate();
    check(menu.isAvailableOn("2026-05-01") && !menu.isAvailableOn("2028-01-01"), "период действия меню");
    check(menu.findDishById(100) != nullptr && menu.findDrinkById(200) == nullptr, "поиск блюда и напитка");
    check(menu.searchItems("пицца").size() == 1, "поиск позиций меню");
    check(menu.countAvailableItems() == 0, "скрытый раздел не учитывается");
    menu.archive();
    check(menu.status() == MenuStatus::Archived && !menu.isAvailableOn("2026-05-01"), "архив меню");
    menu.removeSection(1);
    expectThrow<RestaurantException>([&] { menu.removeSection(1); }, "удаление отсутствующего раздела");
}

void testCustomers() {
    CustomerProfile profile("123", "mail@example.com", "2000-02-02");
    check(profile.contactIsValid(), "контакт клиента");
    profile.addPreference("vegetarian");
    check(profile.hasPreference("vegetarian"), "предпочтение клиента");
    profile.removePreference("vegetarian");
    check(!profile.hasPreference("vegetarian"), "удаление предпочтения");
    profile.recordVisit();
    check(profile.visitCount() == 1, "учёт посещения");
    check(!CustomerProfile("", "bad", "").contactIsValid(), "некорректный контакт");

    LoyaltyCard card("LOYAL-1");
    card.addPoints(350);
    check(card.tier() == "Silver" && card.redeemPoints(50), "начисление и списание бонусов");
    check(!card.redeemPoints(9999), "недостаточно бонусов");
    card.deactivate();
    check(!card.isActive(), "деактивация карты");
    expectThrow<RestaurantException>([&] { card.addPoints(1); }, "начисление на неактивную карту");
    card.activate();
    card.addPoints(700);
    check(card.tier() == "Gold", "уровень Gold");

    Customer customer = makeCustomer();
    check(customer.canMakeReservation() && customer.isActive(), "клиент может бронировать");
    customer.recordVisit();
    customer.deactivate();
    check(!customer.canMakeReservation(), "деактивированный клиент");
    customer.activate();

    RestaurantTable table(1, 1, 4, false, 1);
    Reservation reservation(1, customer, table.id(), 3, "2026-11-26", 720, 90);
    Reservation overlap(2, customer, table.id(), 2, "2026-11-26", 780, 60);
    check(reservation.overlaps(overlap) && reservation.isForCustomer(customer.id()), "пересечение бронирований");
    reservation.confirm(table);
    check(reservation.status() == ReservationStatus::Confirmed && table.status() == TableStatus::Reserved,
          "подтверждение бронирования резервирует столик");
    reservation.markSeated(table);
    check(reservation.status() == ReservationStatus::Seated && table.status() == TableStatus::Occupied,
          "посадка клиента занимает столик");
    reservation.complete(table);
    check(reservation.status() == ReservationStatus::Completed && table.status() == TableStatus::NeedsCleaning,
          "завершение бронирования переводит столик на уборку");
    check(!reservation.overlaps(overlap), "завершённое бронирование не блокирует интервал");
    table.markCleaned();
    check(table.status() == TableStatus::Available, "после уборки столик снова свободен");
    expectThrow<InvalidReservationException>([&] { reservation.cancel(); }, "отмена завершённого бронирования");
    RestaurantTable cancellationTable(4, 4, 2, false, 1);
    Reservation confirmedForCancellation(4, customer, cancellationTable.id(), 2,
                                          "2026-11-27", 600, 60);
    confirmedForCancellation.confirm(cancellationTable);
    check(cancellationTable.status() == TableStatus::Reserved, "подтверждённое бронирование резервирует столик");
    confirmedForCancellation.cancel(cancellationTable);
    check(confirmedForCancellation.status() == ReservationStatus::Cancelled &&
          cancellationTable.status() == TableStatus::Available,
          "отмена бронирования освобождает столик");

    Reservation pending(3, customer, table.id(), 2, "2026-11-27", 600, 60);
    pending.cancel();
    check(pending.status() == ReservationStatus::Cancelled, "отмена бронирования");
    expectThrow<InvalidReservationException>([&] { pending.confirm(table); }, "подтверждение отменённого бронирования");

    WaitlistEntry entry(1, customer, 3, "2026-11-27", 600);
    check(entry.matchesTable(table) && entry.belongsToCustomer(customer.id()), "запись листа ожидания");
    entry.notifyAvailability(table);
    entry.removeFromWaitlist();
    check(!entry.isActive() && !entry.matchesTable(table), "удаление из листа ожидания");
}

void testOrders() {
    Customer customer = makeCustomer();
    Waiter waiter(1, "Официант", 10.0, "2025-01-01");
    MenuItem pizza(101, "Пицца", "Сырная", 10.0, 15);
    OrderItem row(pizza, 2);
    check(closeTo(row.lineTotal(), 20.0) && row.refersTo(pizza), "позиция заказа");
    row.changeQuantity(3);
    row.changeNote("Без лука");
    check(row.note() == "Без лука", "примечание к позиции заказа");
    expectThrow<InvalidOrderException>([&] { row.changeQuantity(0); }, "нулевое количество заказа");

    Order order(1, customer, 1, "2026-11-26 12:00");
    order.assignWaiter(waiter);
    check(order.waiterId() == waiter.id(), "назначение официанта");
    order.addItem(pizza, 2);
    order.addItem(pizza, 1);
    check(order.containsItem(101) && closeTo(order.calculateTotal(), 30.0) && order.createdAt() == "2026-11-26 12:00", "добавление позиции и итог заказа");
    order.changeItemQuantity(101, 2);
    check(closeTo(order.calculateTotal(), 20.0), "изменение количества");
    order.changeStatus(OrderStatus::Accepted, "12:01", waiter.id());
    check(order.history().size() == 1 && order.history()[0].changedBy(waiter.id()), "история статусов");
    expectThrow<DishNotFoundException>([&] { order.removeItem(999); }, "удаление отсутствующей позиции заказа");
    order.removeItem(101);
    expectThrow<InvalidOrderException>([&] { order.changeStatus(OrderStatus::Preparing, "12:02"); }, "изменение пустого заказа");
    order.addItem(pizza, 1);
    order.changeStatus(OrderStatus::Preparing, "12:02");
    order.changeStatus(OrderStatus::Ready, "12:15");
    order.changeStatus(OrderStatus::Served, "12:20");
    check(order.status() == OrderStatus::Served, "переход статуса заказа");

    RestaurantTable reservedTable(20, 20, 4, false, 1);
    Reservation linkedReservation(20, customer, reservedTable.id(), 2, "2026-11-26", 720, 90);
    linkedReservation.confirm(reservedTable);
    Order linkedOrder(20, customer, 0, "2026-11-26 12:00");
    linkedOrder.addItem(pizza, 1);
    linkedOrder.attachReservation(linkedReservation);
    check(linkedOrder.reservationId() == linkedReservation.id() &&
          linkedOrder.tableId() == reservedTable.id(), "связь заказа с бронированием");
    linkedReservation.markSeated(reservedTable);
    check(linkedReservation.status() == ReservationStatus::Seated &&
          reservedTable.status() == TableStatus::Occupied, "посадка по бронированию");
    expectThrow<PaymentFailedException>([&] {
        CashPayment insufficient(20, linkedOrder.calculateTotal(), 1.0);
    }, "недостаточная сумма не завершает бронирование");
    check(linkedReservation.status() == ReservationStatus::Seated &&
          reservedTable.status() == TableStatus::Occupied,
          "неудачная оплата сохраняет бронь и занятый столик");
    CashPayment successfulPayment(21, linkedOrder.calculateTotal(), 12.0);
    check(successfulPayment.process(), "успешная оплата связанного заказа");
    linkedReservation.complete(reservedTable);
    linkedOrder.changeStatus(OrderStatus::Paid, "12:30");
    check(linkedOrder.status() == OrderStatus::Paid &&
          linkedReservation.status() == ReservationStatus::Completed &&
          reservedTable.status() == TableStatus::NeedsCleaning,
          "успешная оплата завершает бронирование и требует уборки столика");
    reservedTable.markCleaned();
    check(reservedTable.status() == TableStatus::Available, "столик доступен после уборки");
    expectThrow<InvalidOrderException>([&] {
        linkedOrder.attachReservation(linkedReservation);
    }, "нельзя повторно привязать бронь к закрытому заказу");

    Bill bill(1, order);
    check(closeTo(bill.subtotal(), 10.0) && bill.lines()[0].description() == "Пицца", "создание счёта");
    bill.setServiceCharge(10.0);
    bill.applyDiscount(1.0);
    check(closeTo(bill.calculateTotal(), 10.0), "расчёт счёта с надбавкой и скидкой");
    bill.close();
    check(bill.isClosed(), "закрытие счёта");
    expectThrow<RestaurantException>([&] { bill.applyDiscount(100.0); }, "чрезмерная скидка");
    expectThrow<RestaurantException>([&] { bill.setServiceCharge(110.0); }, "неверная сервисная надбавка");

    CashPayment cash(1, bill.calculateTotal(), 15.0);
    check(cash.process() && closeTo(cash.change(), 5.0), "оплата наличными");
    cash.reverse();
    check(!cash.processed(), "отмена отметки оплаты");
    expectThrow<PaymentFailedException>([&] { cash.reverse(); }, "повторная отмена платежа");
    CardPayment card(2, 10.0, "****1234");
    check(!card.process(), "карта без подтверждения");
    card.approve("AUTH-1");
    check(card.process() && card.isApproved() && card.method() == "card" && card.authorizationCode() == "AUTH-1", "оплата картой");
    Refund refund(1, card, 4.0, "Ошибка заказа");
    check(refund.complete() && !refund.complete() && refund.isCompleted() && refund.belongsToPayment(card.id()) && refund.reason() == "Ошибка заказа", "возврат оплаты");
    expectThrow<PaymentFailedException>([&] { Refund invalid(2, card, 20.0, "Слишком много"); }, "возврат больше платежа");

    Discount discount(1, "Скидка 10%", 10.0, 5.0);
    check(closeTo(discount.calculateDiscount(100.0), 10.0) && discount.appliesTo(10.0) && discount.name() == "Скидка 10%", "расчёт скидки");
    discount.deactivate();
    check(closeTo(discount.calculateDiscount(100.0), 0.0), "неактивная скидка");
    discount.activate();
    Coupon coupon("SAVE", discount, "2026-12-31", 1);
    check(coupon.isValid("2026-11-26") && closeTo(coupon.calculateDiscount(100, "2026-11-26"), 10.0), "проверка купона");
    check(coupon.use("2026-11-26") && !coupon.isValid("2026-11-26"), "использование купона");
    coupon.deactivate();
    check(!coupon.isValid("2026-11-26"), "деактивация купона");
    expectThrow<PaymentFailedException>([] { CashPayment bad(5, 10.0, 2.0); }, "недостаточно наличных");
}

void testStaffAndKitchen() {
    Employee employee(1, "Сотрудник", "cleaner", 8.0, "2025-01-01");
    check(employee.canPerform("cleaner") && !employee.canPerform("cook"), "роль сотрудника");
    check(closeTo(employee.calculatePay(120), 16.0), "расчёт оплаты труда");
    employee.changeHourlyRate(9.0);
    employee.deactivate();
    check(!employee.isActive(), "деактивация сотрудника");
    employee.activate();

    Waiter waiter(2, "Официант", 10.0, "2025-01-01");
    for (int i = 0; i < 8; ++i) check(waiter.assignTable(), "назначение столика официанту");
    check(!waiter.assignTable(), "лимит столиков официанта");
    waiter.releaseTable();
    waiter.recordCompletedOrder();
    check(waiter.completedOrders() == 1 && waiter.canPerform("serve"), "работа официанта");

    Chef chef(3, "Повар", 12.0, "2024-01-01");
    chef.addSkill("grill");
    chef.addSkill("grill");
    check(chef.hasSkill("grill") && chef.canPerform("cook"), "навыки повара");
    chef.setAvailable(false);
    check(!chef.canPerform("cook"), "занятый повар");
    chef.setAvailable(true);
    chef.completeTask();
    check(chef.completedTasks() == 1, "завершение задачи поваром");

    Administrator admin(4, "Администратор", 15.0, "2023-01-01");
    admin.addManagedEmployee();
    admin.setFinancePermission(false);
    check(admin.managedEmployees() == 1 && !admin.canPerform("manage_finance"), "права администратора");
    admin.removeManagedEmployee();
    check(admin.canPerform("manage_staff"), "управление персоналом");

    WorkShift morning(1, 2, "2026-11-26", 480, 960);
    WorkShift overlap(2, 2, "2026-11-26", 900, 1200);
    check(morning.overlaps(overlap) && morning.includesMinute(500) && !morning.includesMinute(1000), "проверка смены");
    morning.markCompleted();
    check(morning.isCompleted() && morning.durationMinutes() == 480, "завершение смены");
    WorkSchedule schedule;
    schedule.addShift(morning);
    expectThrow<ShiftConflictException>([&] { schedule.addShift(overlap); }, "пересечение смен");
    check(schedule.findShift(1) != nullptr && schedule.totalMinutesForEmployee(2) == 480, "поиск и сумма смен");
    check(schedule.isWithinWeeklyLimit(2, 300) && !schedule.isWithinWeeklyLimit(2, 2000), "недельный лимит");
    check(schedule.hasRestPeriod(WorkShift(3, 2, "2026-11-27", 600, 1200)), "междусменный отдых в модели");
    check(!schedule.hasRestPeriod(WorkShift(4, 2, "2026-11-26", 1000, 1300)), "недостаточный отдых между сменами");
    schedule.setWeeklyLimit(600);
    schedule.removeShift(1);
    check(schedule.shifts().empty(), "удаление смены");
    expectThrow<RestaurantException>([&] { schedule.setWeeklyLimit(0); }, "нулевой лимит");

    AttendanceRecord attendance(1, 2, 1);
    attendance.clockIn(490);
    check(attendance.isLate(480), "опоздание");
    attendance.clockOut(970);
    check(attendance.isComplete() && attendance.workedMinutes() == 480, "учёт рабочего времени");
    attendance.addNote("Смена завершена");

    KitchenStation station(1, "Горячий цех", "all");
    check(station.supports("grill") && station.assignTask(1, "grill"), "назначение задачи станции");
    check(!station.assignTask(2, "grill") && station.handlesTask(1), "занятая станция");
    station.releaseTask(1);
    check(!station.isBusy(), "освобождение станции");

    CookingTask task(1, 1, 101, "Паста", 10, "all");
    check(task.assignChef(chef) && task.assignStation(station), "назначение повара и станции");
    task.start();
    check(task.isOverdue(11), "просроченная задача");
    task.complete();
    check(task.status() == CookingStatus::Ready, "готовность блюда");
    CookingTask cancelled(2, 1, 102, "Салат", 5);
    cancelled.cancel();
    check(cancelled.status() == CookingStatus::Cancelled, "отмена задачи");
    expectThrow<RestaurantException>([&] { cancelled.cancel(); }, "проверка отменённой задачи");

    Kitchen kitchen(1, "Главная кухня");
    kitchen.addStation(station);
    kitchen.enqueueTask(CookingTask(3, 2, 103, "Суп", 8));
    check(kitchen.countPendingTasks() == 1 && kitchen.findTask(3) != nullptr, "очередь кухни");
    kitchen.stopAcceptingOrders();
    expectThrow<RestaurantClosedException>([&] { kitchen.enqueueTask(CookingTask(4, 2, 104, "Салат", 3)); }, "закрытая кухня");
    kitchen.resumeAcceptingOrders();
    kitchen.addStation(KitchenStation(2, "Универсальная", "all"));
    check(kitchen.assignTask(3, chef, 2), "распределение задачи кухней");
    kitchen.finishTask(3);
    check(kitchen.countPendingTasks() == 0, "завершение кухонной задачи");
}

void testInventoryAndDelivery() {
    Ingredient tomato(1, "Томат", "кг", 3.0, 18.0, 10);
    StockItem stock(tomato, 10.0, 2.0, "Холодильник", "LOT-1");
    check(stock.receive(2.0) && closeTo(stock.totalValue(), 36.0), "поступление и стоимость запаса");
    check(stock.reserve(3.0) && closeTo(stock.availableQuantity(), 9.0), "резервирование запаса");
    stock.releaseReservation(1.0);
    check(stock.consume(2.0) && stock.needsRestock() == false, "расход ингредиента");
    check(stock.isSameIngredient(tomato), "сопоставление продукта");
    expectThrow<InsufficientStockException>([&] { stock.releaseReservation(9.0); }, "снятие избыточного резерва");

    StockMovement receipt(1, tomato.id(), 5.0, MovementType::Receipt, "Поставка", "2026-11-26");
    StockMovement consumption(2, tomato.id(), 1.0, MovementType::Consumption, "Приготовление", "2026-11-26");
    check(receipt.isIncoming() && closeTo(consumption.signedQuantity(), -1.0) && consumption.concernsIngredient(tomato), "движение склада");
    check(StockMovement(3, 1, 1, MovementType::Waste, "spoiled", "2026-11-26").isWaste(), "списание в движении");

    Inventory inventory(1, "Основной склад", 100.0);
    inventory.addStockItem(stock);
    check(inventory.isCapacityAvailable(20.0), "свободная ёмкость склада");
    check(inventory.receive(1, 5.0, receipt), "приёмка запаса на склад");
    check(inventory.consume(1, 1.0, consumption), "расход со склада");
    check(closeTo(inventory.calculateTotalValue(), inventory.findItem(1)->totalValue()), "оценка склада");
    check(inventory.itemsToRestock().empty(), "список пополнения");
    expectThrow<InsufficientStockException>([&] { inventory.consume(1, 1000.0, consumption); }, "недостаток на складе");
    expectThrow<IngredientUnavailableException>([&] { inventory.consume(99, 1.0, consumption); }, "отсутствующий ингредиент");
    inventory.removeStockItem(1);
    check(inventory.items().empty(), "удаление складской позиции");

    Supplier supplier(1, "Поставщик", "Менеджер", "123");
    supplier.addIngredient(tomato);
    check(supplier.supplies(tomato) && supplier.isActive(), "ассортимент поставщика");
    supplier.updateContact("Новый менеджер", "456");
    supplier.deactivate();
    check(!supplier.isActive(), "деактивация поставщика");
    supplier.activate();

    PurchaseOrderItem purchaseItem(tomato, 10.0, 3.0);
    check(closeTo(purchaseItem.totalCost(), 30.0) && closeTo(purchaseItem.pendingQuantity(), 10.0), "позиция закупки");
    purchaseItem.receive(4.0);
    check(!purchaseItem.isComplete() && closeTo(purchaseItem.pendingQuantity(), 6.0), "частичная приёмка");
    PurchaseOrder purchase(1, supplier, "2026-11-26");
    purchase.addItem(PurchaseOrderItem(tomato, 5.0, 3.0));
    check(closeTo(purchase.calculateTotal(), 15.0), "сумма закупки");
    purchase.addNote("Срочная поставка");
    purchase.send();
    purchase.receiveItem(0, 2.0);
    check(purchase.status() == PurchaseOrderStatus::PartiallyReceived, "частичное получение закупки");
    purchase.receiveItem(0, 3.0);
    check(purchase.allItemsReceived() && purchase.status() == PurchaseOrderStatus::Received, "получение всей закупки");
    expectThrow<RestaurantException>([&] { purchase.cancel(); }, "отмена полученной закупки");

    GoodsReceipt goodsReceipt(1, purchase, "2026-11-26");
    goodsReceipt.recordMovement(receipt);
    check(closeTo(goodsReceipt.receivedQuantity(), 5.0) && goodsReceipt.belongsToOrder(purchase.id()), "накладная приёмки");
    goodsReceipt.confirm();
    check(goodsReceipt.isConfirmed(), "подтверждение приёмки");
    expectThrow<RestaurantException>([&] { goodsReceipt.recordMovement(receipt); }, "изменение подтверждённой приёмки");

    WasteRecord waste(1, tomato, 2.0, "spoiled", "2026-11-26");
    check(closeTo(waste.calculateLoss(), 6.0) && waste.isSpoilage() && waste.concerns(tomato), "учёт списания");
    waste.changeReason("expired");
    check(waste.isSpoilage(), "изменение причины списания");

    DeliveryAddress address("Ленина", "1", "Минск", "10", "Позвонить");
    check(address.isValid() && address.isInCity("Минск") && address.format().find("10") != std::string::npos, "адрес доставки");
    address.updateInstructions("Оставить у двери");
    check(!DeliveryAddress().isValid(), "проверка пустого адреса доставки");
    Courier courier(1, "Курьер", "555");
    check(courier.assignDelivery() && !courier.isAvailable(), "назначение курьера");
    courier.completeDelivery();
    courier.addRating(4.0);
    check(courier.isAvailable() && courier.completedDeliveries() == 1 && closeTo(courier.rating(), 4.0), "завершение доставки курьером");
    expectThrow<RestaurantException>([&] { courier.addRating(6.0); }, "некорректная оценка курьера");

    Customer customer = makeCustomer();
    MenuItem item(101, "Суп", "Овощной суп", 5.0, 5);
    Order order(10, customer, 0, "2026-11-26");
    order.addItem(item, 1);
    Delivery delivery(1, order, customer, address, 4.0, "2026-11-26 12:00");
    check(delivery.assignCourier(courier) && delivery.courierId() == courier.id() && !courier.isAvailable() && delivery.createdAt() == "2026-11-26 12:00" && delivery.address().isValid(), "назначение курьера доставке");
    delivery.markPickedUp();
    delivery.complete(courier);
    check(delivery.status() == DeliveryStatus::Delivered && delivery.belongsToCustomer(customer.id()), "завершение доставки");
    expectThrow<DeliveryUnavailableException>([&] { delivery.cancel(); }, "отмена завершённой доставки");
    Delivery cancelledDelivery(2, order, customer, address, 4.0, "2026-11-26 12:00");
    cancelledDelivery.cancel();
    check(!cancelledDelivery.isActive(), "отмена доставки");
}

void testReports() {
    Customer customer = makeCustomer();
    MenuItem item(101, "Суп", "Суп дня", 5.0, 5);
    Order order(1, customer, 0, "2026-11-26");
    order.addItem(item, 2);
    order.changeStatus(OrderStatus::Accepted, "12:01");
    Bill bill(1, order);
    bill.applyDiscount(1.0);
    SalesReport sales("2026-11-01", "2026-11-30");
    sales.recordSale(bill);
    CardPayment payment(1, 9.0, "****1111");
    payment.approve("OK");
    payment.process();
    Refund refund(1, payment, 2.0, "Возврат");
    refund.complete();
    sales.recordRefund(refund);
    check(sales.hasSales() && sales.orderCount() == 1 && closeTo(sales.netRevenue(), 7.0), "отчёт о продажах");
    check(closeTo(sales.averageCheck(), 7.0), "средний чек");

    Ingredient ingredient(1, "Картофель", "кг", 2.0, 80.0, 20);
    Inventory inventory(1, "Склад", 100.0);
    inventory.addStockItem(StockItem(ingredient, 10.0, 2.0, "A1", "LOT"));
    InventoryReport inventoryReport("2026-11-26");
    inventoryReport.updateFrom(inventory);
    check(inventoryReport.itemCount() == 1 && !inventoryReport.hasLowStock(), "отчёт склада");
    check(closeTo(inventoryReport.averageItemValue(), 20.0) && closeTo(inventoryReport.totalQuantity(), 10.0), "стоимость и количество склада");

    MenuPerformanceReport menuReport("Ноябрь");
    menuReport.recordSale(item, 3);
    menuReport.recordSale(item, 2);
    menuReport.recordUnavailableEvent();
    check(menuReport.quantitySold(101) == 5 && menuReport.quantitySold(999) == 0, "продажи позиции меню");
    check(menuReport.popularItemId() == 101 && closeTo(menuReport.revenue(), 25.0), "популярное блюдо и выручка");
    check(menuReport.totalItemsSold() == 5 && menuReport.unavailableEvents() == 1, "итоги отчёта меню");
    expectThrow<RestaurantException>([&] { menuReport.recordSale(item, 0); }, "нулевое количество продаж");

    Chef chef(5, "Повар", 12.0, "2024-01-01");
    StaffPerformanceReport staffReport("Ноябрь");
    staffReport.recordTask(chef);
    staffReport.recordWorkedTime(chef, 300);
    staffReport.recordLateArrival();
    staffReport.recordAbsence();
    check(staffReport.tasksCompleted(chef.id()) == 1 && staffReport.workedMinutes(chef.id()) == 300, "показатели сотрудника");
    check(staffReport.hasRecords() && staffReport.punctualityIncidents() == 2, "пунктуальность сотрудника");
    expectThrow<RestaurantException>([&] { staffReport.recordWorkedTime(chef, -1); }, "отрицательное рабочее время");

    RestaurantStatistics statistics;
    statistics.recordCustomer(customer);
    statistics.recordOrder(order);
    statistics.recordPayment(payment);
    statistics.recordRefund(refund);
    Courier courier(1, "Курьер", "123");
    DeliveryAddress address("А", "1", "Минск", "", "");
    Delivery delivery(1, order, customer, address, 2.0, "12:00");
    delivery.assignCourier(courier);
    delivery.markPickedUp();
    delivery.complete(courier);
    statistics.recordDelivery(delivery);
    check(statistics.totalOrders() == 1 && statistics.completedOrders() == 0, "общая статистика заказов");
    check(statistics.completedDeliveries() == 1 && closeTo(statistics.netRevenue(), 7.0), "выручка и доставки");
    check(closeTo(statistics.averageOrderValue(), 9.0) && closeTo(statistics.cancellationRate(), 0.0), "средняя сумма и отмены");
}

} // namespace

int main() {
    try {
        testCore();
        testMenu();
        testCustomers();
        testOrders();
        testStaffAndKitchen();
        testInventoryAndDelivery();
        testReports();
        std::cout << "Все проверки пройдены. Количество проверок: " << checks << "\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 1;
    }
}