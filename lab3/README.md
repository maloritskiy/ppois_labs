# Лабораторная работа №3 — Система управления рестораном

## Назначение

Программа моделирует работу одного ресторана: управление залами и столиками, меню, клиентами, бронированиями, заказами, оплатой, персоналом, кухней, складом и доставкой. Данные хранятся в памяти во время запуска программы; внешняя база данных и реальные платёжные/доставочные сервисы не используются.

## Состав проекта

```text
lab3/
├── Makefile
├── README.md
├── restaurant_management.hpp
├── restaurant_exceptions.hpp
├── restaurant_core.hpp
├── restaurant_menu.hpp
├── restaurant_customers.hpp
├── restaurant_orders.hpp
├── restaurant_staff.hpp
├── restaurant_kitchen.hpp
├── restaurant_inventory.hpp
├── restaurant_delivery.hpp
├── restaurant_statistics.hpp
├── main.cpp
└── tests.cpp
```

`restaurant_management.hpp` подключает заголовки объектной модели. `main.cpp` содержит CLI, `tests.cpp` — консольные unit-тесты. Файлы предметной модели разделены по подсистемам.

## Количественные показатели

| Показатель | Реализация | Требование |
|---|---:|---:|
| Предметные классы | 56 | не менее 50 |
| Поля предметных классов | 307 | не менее 150 |
| Содержательные методы | более 200 | не менее 100 |
| Ассоциации между классами | 63 уникальные связи | не менее 30 |
| Собственные исключения | 13 специализированных + базовый | не менее 12 |
| Unit-тесты | 235 проверок; gcov: около 96,1% строк заголовков модели | покрытие более 90% |

Поля подсчитаны по объявлениям членов данных в 56 предметных классах. При подсчёте содержательных методов не учитываются конструкторы, деструкторы и простые методы доступа. 63 ассоциации — это уникальные пары классов, связанные типизированными полями, контейнерами или параметрами методов; поля, содержащие только числовые идентификаторы, сами по себе ассоциацией не считаются. Наследование учитывается отдельно. Локальная проверка `gcov` показала около 96,1% покрытия строк десяти заголовков предметной модели; цель `make coverage` дополнительно проверяет порог 90,01% с помощью `gcovr` (этот инструмент устанавливается отдельно).

## Описание объектной модели

В таблицах перечислены поля и публичные методы классов. Для компактности поля записаны как `имя (тип)`, а методы — по именам; конструкторы не перечисляются. Ассоциации указаны в последнем столбце.

### Restaurant Core

| Класс | Назначение | Поля | Методы | Ассоциации |
|---|---|---|---|---|
| `RestaurantAddress` | Хранит почтовый адрес ресторана. | `street` (std::string), `buildingNumber` (std::string), `city` (std::string), `postalCode` (std::string), `country` (std::string) | `city()`, `isValid()`, `formatFullAddress()`, `isLocatedIn()` | — |
| `OpeningHours` | Описывает рабочий интервал ресторана на один день недели. | `dayOfWeek` (std::string), `openingMinute` (int), `closingMinute` (int), `closed` (bool), `specialNote` (std::string) | `day()`, `specialNote()`, `setSpecialNote()`, `isClosed()`, `isValid()`, `isOpenAt()`, `getDurationMinutes()`, `setWorkingPeriod()`, `closeForDay()` | — |
| `TableZone` | Описывает зону размещения столиков. | `zoneId` (int), `name` (std::string), `floorNumber` (int), `outdoor` (bool), `quiet` (bool), `features` (std::vector<std::string>) | `id()`, `name()`, `floor()`, `matchesPreferences()`, `hasFeature()`, `addFeature()`, `removeFeature()` | — |
| `RestaurantTable` | Хранит состояние, номер и вместимость столика. | `tableId` (int), `tableNumber` (int), `capacity` (int), `status` (TableStatus), `nearWindow` (bool), `zoneId` (int) | `id()`, `number()`, `capacity()`, `zoneId()`, `isNearWindow()`, `status()`, `canAccommodate()`, `reserve()`, `occupy()`, `release()`, `markNeedsCleaning()`, `markCleaned()`, `takeOutOfService()`, `restoreService()`, `matchesPreferences()`, `assignZone()` | — (зона определяется по zoneId в DiningHall) |
| `DiningHall` | Объединяет столики и зоны отдельного зала. | `hallId` (int), `name` (std::string), `floorNumber` (int), `capacity` (int), `open` (bool), `tables` (std::vector<RestaurantTable>), `zones` (std::vector<TableZone>) | `id()`, `name()`, `floor()`, `capacity()`, `tables()`, `addTable()`, `removeTable()`, `addZone()`, `assignTableToZone()`, `findTable()`, `findTableById()`, `findAvailableTables()`, `calculateFreeSeats()`, `open()`, `close()`, `isOpen()` | RestaurantTable, TableZone |
| `Restaurant` | Главный объект системы: адрес, график, залы и действующее меню. | `restaurantId` (int), `name` (std::string), `phoneNumber` (std::string), `address` (RestaurantAddress), `openingHours` (std::vector<OpeningHours>), `halls` (std::vector<DiningHall>), `activeMenu` (Menu) | `id()`, `name()`, `phoneNumber()`, `address()`, `halls()`, `activeMenu()`, `addHall()`, `removeHall()`, `findTable()`, `findTableById()`, `isOpenAt()`, `updateOpeningHours()`, `setActiveMenu()`, `calculateTotalCapacity()`, `findAvailableTables()` | RestaurantAddress, OpeningHours, DiningHall, Menu |

### Menu Management

| Класс | Назначение | Поля | Методы | Ассоциации |
|---|---|---|---|---|
| `MenuItem` | Общие данные позиции меню: название, описание, цена и доступность. | `itemId` (int), `name` (std::string), `description` (std::string), `price` (double), `preparationTimeMinutes` (int), `available` (bool) | `id()`, `name()`, `description()`, `price()`, `preparationTime()`, `available()`, `changePrice()`, `markAvailable()`, `markUnavailable()`, `matchesSearch()`, `canBeOrdered()` | — (MenuItem используется как поле в Dish, Drink и OrderItem) |
| `Ingredient` | Описывает ингредиент, его стоимость, калорийность и аллергены. | `ingredientId` (int), `name` (std::string), `unit` (std::string), `purchasePricePerUnit` (double), `caloriesPerUnit` (double), `allergens` (std::vector<std::string>), `expiryDays` (int) | `id()`, `name()`, `unit()`, `purchasePrice()`, `caloriesPerUnit()`, `allergens()`, `calculateCost()`, `calculateCalories()`, `containsAllergen()`, `addAllergen()`, `removeAllergen()`, `expiresWithin()`, `isSuitableForDiet()` | RecipeIngredient, Recipe, StockItem, Supplier, StockMovement, PurchaseOrderItem, WasteRecord |
| `RecipeIngredient` | Хранит количество одного ингредиента в рецепте. | `ingredientId` (int), `quantity` (double), `unit` (std::string), `optional` (bool), `preparationNote` (std::string) | `ingredientId()`, `quantity()`, `unit()`, `optional()`, `preparationNote()`, `calculateRequiredQuantity()`, `calculateCost()`, `matchesIngredient()`, `calculateQuantityWithWaste()` | Ingredient |
| `Recipe` | Содержит инструкции, время приготовления и состав блюда. | `recipeId` (int), `name` (std::string), `instructions` (std::string), `basePortions` (int), `preparationTimeMinutes` (int), `ingredients` (std::vector<RecipeIngredient>) | `id()`, `name()`, `instructions()`, `preparationTime()`, `ingredients()`, `addIngredient()`, `removeIngredient()`, `containsIngredient()`, `calculateTotalCost()`, `calculateCostPerPortion()`, `scaleIngredients()`, `containsAllergen()`, `isComplete()` | RecipeIngredient, Ingredient |
| `Dish` | Представляет блюдо и связывает позицию меню с рецептом. | `menuItem` (MenuItem), `recipe` (Recipe), `category` (std::string), `caloriesPerPortion` (double), `vegetarian` (bool), `spicy` (bool) | `id()`, `menuItem()`, `recipe()`, `category()`, `caloriesPerPortion()`, `calculateCost()`, `isSuitableForDiet()`, `containsAllergen()`, `updateRecipe()`, `calculateProfit()`, `isSpicy()`, `isVegetarian()` | MenuItem, Recipe, Ingredient |
| `Drink` | Представляет напиток и его объём/алкогольные характеристики. | `menuItem` (MenuItem), `drinkType` (std::string), `volumeMl` (int), `alcoholic` (bool), `alcoholPercentage` (double) | `id()`, `menuItem()`, `drinkType()`, `volumeMl()`, `isAlcoholFree()`, `calculatePureAlcoholVolume()`, `isSuitableForChildren()`, `changeVolume()` | MenuItem |
| `MenuSection` | Объединяет блюда и напитки в один раздел меню. | `sectionId` (int), `title` (std::string), `description` (std::string), `displayOrder` (int), `visible` (bool), `dishes` (std::vector<Dish>), `drinks` (std::vector<Drink>) | `id()`, `title()`, `description()`, `displayOrder()`, `dishes()`, `drinks()`, `addDish()`, `addDrink()`, `removeItem()`, `findDishById()`, `findDrinkById()`, `countAvailableItems()`, `sortItemsByPrice()`, `setVisible()`, `visible()` | Dish, Drink |
| `Menu` | Содержит разделы и управляет состоянием и сроком действия меню. | `menuId` (int), `name` (std::string), `description` (std::string), `status` (MenuStatus), `sections` (std::vector<MenuSection>), `validFrom` (std::string), `validUntil` (std::string) | `id()`, `name()`, `description()`, `validFrom()`, `validUntil()`, `status()`, `sections()`, `addSection()`, `removeSection()`, `findSection()`, `activate()`, `archive()`, `isAvailableOn()`, `findDishById()`, `findDrinkById()`, `searchItems()`, `countAvailableItems()` | MenuSection, Dish, Drink |

### Customer Management

| Класс | Назначение | Поля | Методы | Ассоциации |
|---|---|---|---|---|
| `CustomerProfile` | Хранит контактные данные, дату рождения и предпочтения клиента. | `phoneNumber` (std::string), `email` (std::string), `birthDate` (std::string), `preferences` (std::vector<std::string>), `visitCount` (int) | `phoneNumber()`, `email()`, `birthDate()`, `addPreference()`, `removePreference()`, `hasPreference()`, `recordVisit()`, `visitCount()`, `contactIsValid()` | Customer |
| `LoyaltyCard` | Управляет бонусными баллами и уровнем программы лояльности. | `cardNumber` (std::string), `points` (int), `tier` (std::string), `active` (bool), `totalPointsEarned` (int) | `number()`, `points()`, `tier()`, `addPoints()`, `redeemPoints()`, `deactivate()`, `activate()`, `isActive()`, `updateTier()` | Customer |
| `Customer` | Представляет клиента ресторана и объединяет профиль с бонусной картой. | `customerId` (int), `fullName` (std::string), `profile` (CustomerProfile), `loyaltyCard` (LoyaltyCard), `registrationDate` (std::string), `active` (bool) | `id()`, `name()`, `registrationDate()`, `profile()`, `loyaltyCard()`, `deactivate()`, `activate()`, `isActive()`, `canMakeReservation()`, `recordVisit()` | CustomerProfile, LoyaltyCard |
| `Reservation` | Хранит бронирование; отслеживает подтверждение, посадку и завершение посещения. | `reservationId` (int), `customer` (Customer), `tableId` (int), `guestCount` (int), `date` (std::string), `startMinute` (int), `durationMinutes` (int), `status` (ReservationStatus) | `id()`, `tableId()`, `date()`, `startMinute()`, `durationMinutes()`, `guestCount()`, `status()`, `customer()`, `overlaps()`, `confirm()`, `markSeated()`, `cancel()`, `cancel(RestaurantTable&)`, `complete(RestaurantTable&)`, `isForCustomer()` | Customer, RestaurantTable |
| `WaitlistEntry` | Представляет запись клиента в лист ожидания. | `entryId` (int), `customer` (Customer), `guestCount` (int), `preferredDate` (std::string), `preferredStartMinute` (int), `note` (std::string), `active` (bool) | `id()`, `guestCount()`, `preferredDate()`, `preferredStartMinute()`, `note()`, `isActive()`, `matchesTable()`, `notifyAvailability()`, `removeFromWaitlist()`, `belongsToCustomer()` | Customer, RestaurantTable |

### Order Management

| Класс | Назначение | Поля | Методы | Ассоциации |
|---|---|---|---|---|
| `OrderItem` | Одна позиция заказа с количеством и зафиксированной ценой. | `menuItem` (MenuItem), `quantity` (int), `unitPrice` (double), `note` (std::string) | `itemId()`, `name()`, `note()`, `quantity()`, `unitPrice()`, `lineTotal()`, `changeQuantity()`, `changeNote()`, `refersTo()` | MenuItem |
| `OrderStatusHistory` | Хранит запись об изменении состояния заказа. | `status` (OrderStatus), `timestamp` (std::string), `note` (std::string), `employeeId` (int) | `status()`, `timestamp()`, `changedBy()`, `hasNote()` | — (сотрудник хранится по employeeId) |
| `Order` | Объединяет клиента, позиции заказа, официанта, историю статусов и необязательную связь с бронированием. | `orderId` (int), `customer` (Customer), `tableId` (int), `reservationId` (int), `items` (std::vector<OrderItem>), `history` (std::vector<OrderStatusHistory>), `status` (OrderStatus), `createdAt` (std::string), `waiterId` (int) | `id()`, `customerId()`, `tableId()`, `reservationId()`, `status()`, `items()`, `createdAt()`, `assignWaiter()`, `waiterId()`, `attachReservation()`, `addItem()`, `removeItem()`, `changeItemQuantity()`, `calculateTotal()`, `changeStatus()`, `cancel()`, `containsItem()`, `history()` | Customer, Reservation, OrderItem, OrderStatusHistory, Waiter |
| `BillLine` | Одна строка счёта с количеством и ценой. | `itemId` (int), `description` (std::string), `quantity` (int), `unitPrice` (double) | `itemId()`, `description()`, `total()`, `changeQuantity()`, `hasSameItem()` | — (позиция определяется по itemId) |
| `Bill` | Формирует сумму к оплате по заказу, сервисному сбору и скидке. | `billId` (int), `orderId` (int), `lines` (std::vector<BillLine>), `serviceChargePercent` (double), `discountAmount` (double), `closed` (bool) | `id()`, `orderId()`, `lines()`, `subtotal()`, `setServiceCharge()`, `applyDiscount()`, `calculateTotal()`, `close()`, `isClosed()` | Order, BillLine |
| `Payment` | Базовая модель платежа. | `paymentId` (int), `amount` (double), `processed` (bool), `method` (std::string) | `id()`, `amount()`, `processed()`, `process()`, `method()`, `reverse()` | — |
| `CashPayment` | Реализует оплату наличными и расчёт сдачи. | `cashReceived` (double), `change` (double) | `process()`, `change()`, `addCash()` | Payment (наследование) |
| `CardPayment` | Реализует моделируемую оплату картой с подтверждением. | `maskedCardNumber` (std::string), `authorizationCode` (std::string), `approved` (bool) | `approve()`, `process()`, `isApproved()`, `authorizationCode()`, `method()` | Payment (наследование) |
| `Refund` | Описывает возврат ранее проведённого платежа. | `refundId` (int), `paymentId` (int), `amount` (double), `reason` (std::string), `completed` (bool) | `complete()`, `isCompleted()`, `amount()`, `reason()`, `belongsToPayment()` | Payment |
| `Discount` | Рассчитывает скидку по проценту и минимальной сумме. | `discountId` (int), `name` (std::string), `percentage` (double), `minimumTotal` (double), `active` (bool) | `calculateDiscount()`, `activate()`, `deactivate()`, `appliesTo()`, `percentage()`, `name()` | Coupon |
| `Coupon` | Представляет код купона с ограничением срока и количества использований. | `code` (std::string), `discount` (Discount), `expiryDate` (std::string), `usesRemaining` (int), `active` (bool) | `isValid()`, `calculateDiscount()`, `use()`, `deactivate()`, `code()` | Discount |

### Staff Management

| Класс | Назначение | Поля | Методы | Ассоциации |
|---|---|---|---|---|
| `Employee` | Базовый класс сотрудника ресторана. | `employeeId` (int), `fullName` (std::string), `role` (std::string), `hourlyRate` (double), `active` (bool), `hireDate` (std::string) | `id()`, `name()`, `role()`, `hireDate()`, `hourlyRate()`, `isActive()`, `canPerform()`, `deactivate()`, `activate()`, `changeHourlyRate()`, `calculatePay()` | Waiter, Chef, Administrator (наследование) |
| `Waiter` | Сотрудник, обслуживающий столики и заказы. | `assignedTables` (int), `completedOrders` (int), `maximumTables` (int) | `canPerform()`, `assignTable()`, `releaseTable()`, `recordCompletedOrder()`, `completedOrders()`, `assignedTables()` | Employee (наследование), Order |
| `Chef` | Сотрудник кухни с набором навыков и счётчиком задач. | `skills` (std::vector<std::string>), `completedTasks` (int), `available` (bool) | `canPerform()`, `addSkill()`, `hasSkill()`, `setAvailable()`, `completeTask()`, `completedTasks()` | Employee (наследование), CookingTask |
| `Administrator` | Сотрудник с административными полномочиями. | `managedEmployees` (int), `canManageFinance` (bool) | `canPerform()`, `addManagedEmployee()`, `removeManagedEmployee()`, `setFinancePermission()`, `managedEmployees()` | Employee (наследование) |
| `WorkShift` | Описывает рабочую смену одного сотрудника. | `shiftId` (int), `employeeId` (int), `date` (std::string), `startMinute` (int), `endMinute` (int), `completed` (bool) | `id()`, `employeeId()`, `date()`, `durationMinutes()`, `startMinute()`, `endMinute()`, `overlaps()`, `includesMinute()`, `markCompleted()`, `isCompleted()` | — (сотрудник определяется по employeeId) |
| `WorkSchedule` | Хранит смены и проверяет пересечения и лимиты рабочего времени. | `shifts` (std::vector<WorkShift>), `weeklyLimitMinutes` (int), `minimumRestMinutes` (int) | `addShift()`, `removeShift()`, `findShift()`, `totalMinutesForEmployee()`, `isWithinWeeklyLimit()`, `setWeeklyLimit()`, `hasRestPeriod()`, `shifts()` | WorkShift |
| `AttendanceRecord` | Фиксирует время прихода и ухода сотрудника. | `recordId` (int), `employeeId` (int), `shiftId` (int), `checkInMinute` (int), `checkOutMinute` (int), `note` (std::string) | `clockIn()`, `clockOut()`, `workedMinutes()`, `isLate()`, `isComplete()`, `addNote()`, `id()`, `employeeId()`, `shiftId()`, `note()` | — (сотрудник и смена определяются по идентификаторам) |

### Kitchen Management

| Класс | Назначение | Поля | Методы | Ассоциации |
|---|---|---|---|---|
| `KitchenStation` | Описывает рабочую станцию и её текущую занятость. | `stationId` (int), `name` (std::string), `supportedCategory` (std::string), `busy` (bool), `currentTaskId` (int) | `id()`, `name()`, `category()`, `isBusy()`, `supports()`, `assignTask()`, `releaseTask()`, `handlesTask()` | — (текущая задача определяется по currentTaskId) |
| `CookingTask` | Задача на приготовление позиции заказа. | `taskId` (int), `orderId` (int), `orderItemId` (int), `dishName` (std::string), `category` (std::string), `chefId` (int), `stationId` (int), `status` (CookingStatus), `estimatedMinutes` (int) | `id()`, `orderId()`, `orderItemId()`, `dishName()`, `chefId()`, `stationId()`, `category()`, `status()`, `assignChef()`, `assignStation()`, `start()`, `complete()`, `cancel()`, `isOverdue()` | Chef, KitchenStation (параметры методов); OrderItem определяется по идентификатору |
| `Kitchen` | Управляет рабочими станциями и очередью задач кухни. | `kitchenId` (int), `name` (std::string), `stations` (std::vector<KitchenStation>), `tasks` (std::vector<CookingTask>), `acceptingOrders` (bool) | `id()`, `name()`, `addStation()`, `enqueueTask()`, `findTask()`, `findStation()`, `assignTask()`, `finishTask()`, `countPendingTasks()`, `stopAcceptingOrders()`, `resumeAcceptingOrders()` | KitchenStation, CookingTask, Chef |

### Inventory Management

| Класс | Назначение | Поля | Методы | Ассоциации |
|---|---|---|---|---|
| `StockItem` | Хранит остаток, резерв и минимальный запас конкретного ингредиента. | `ingredient` (Ingredient), `quantity` (double), `minimumQuantity` (double), `reservedQuantity` (double), `storageLocation` (std::string), `lotNumber` (std::string) | `ingredientId()`, `ingredient()`, `storageLocation()`, `lotNumber()`, `quantity()`, `availableQuantity()`, `receive()`, `consume()`, `reserve()`, `releaseReservation()`, `needsRestock()`, `totalValue()`, `isSameIngredient()` | Ingredient |
| `StockMovement` | Описывает поступление, расход, списание или корректировку остатка. | `movementId` (int), `ingredientId` (int), `quantity` (double), `type` (MovementType), `reason` (std::string), `date` (std::string) | `id()`, `ingredientId()`, `quantity()`, `isIncoming()`, `signedQuantity()`, `isWaste()`, `reason()`, `date()`, `concernsIngredient()` | Ingredient |
| `Inventory` | Управляет складскими позициями и движениями товаров. | `inventoryId` (int), `name` (std::string), `items` (std::vector<StockItem>), `movements` (std::vector<StockMovement>), `totalCapacity` (double) | `id()`, `name()`, `addStockItem()`, `findItem()`, `removeStockItem()`, `receive()`, `consume()`, `itemsToRestock()`, `calculateTotalValue()`, `usedCapacity()`, `isCapacityAvailable()`, `items()`, `movements()` | StockItem, StockMovement |
| `Supplier` | Хранит поставщика и перечень поставляемых ингредиентов. | `supplierId` (int), `name` (std::string), `contactName` (std::string), `phoneNumber` (std::string), `ingredientIds` (std::vector<int>), `active` (bool) | `id()`, `name()`, `contactName()`, `phoneNumber()`, `addIngredient()`, `supplies()`, `deactivate()`, `activate()`, `isActive()`, `updateContact()` | Ingredient |
| `PurchaseOrderItem` | Описывает количество и стоимость ингредиента в закупке. | `ingredientId` (int), `quantity` (double), `unitPrice` (double), `receivedQuantity` (double) | `ingredientId()`, `quantity()`, `receivedQuantity()`, `totalCost()`, `pendingQuantity()`, `receive()`, `isComplete()` | Ingredient |
| `PurchaseOrder` | Формирует заказ поставщику и отслеживает приёмку. | `purchaseOrderId` (int), `supplier` (Supplier), `items` (std::vector<PurchaseOrderItem>), `orderDate` (std::string), `status` (PurchaseOrderStatus), `note` (std::string) | `id()`, `orderDate()`, `note()`, `status()`, `addItem()`, `calculateTotal()`, `send()`, `receiveItem()`, `allItemsReceived()`, `cancel()`, `addNote()`, `supplier()`, `items()` | Supplier, PurchaseOrderItem |
| `GoodsReceipt` | Фиксирует фактическую приёмку поставки. | `receiptId` (int), `purchaseOrderId` (int), `receiptDate` (std::string), `movements` (std::vector<StockMovement>), `confirmed` (bool) | `recordMovement()`, `confirm()`, `isConfirmed()`, `receiptDate()`, `receivedQuantity()`, `belongsToOrder()` | PurchaseOrder, StockMovement |
| `WasteRecord` | Учитывает списание ингредиента и финансовые потери. | `wasteId` (int), `ingredientId` (int), `quantity` (double), `reason` (std::string), `date` (std::string), `unitCost` (double) | `calculateLoss()`, `isSpoilage()`, `concerns()`, `date()`, `changeReason()` | Ingredient |

### Delivery Management

| Класс | Назначение | Поля | Методы | Ассоциации |
|---|---|---|---|---|
| `DeliveryAddress` | Хранит адрес и инструкции для доставки. | `street` (std::string), `building` (std::string), `city` (std::string), `apartment` (std::string), `instructions` (std::string) | `instructions()`, `isValid()`, `format()`, `isInCity()`, `updateInstructions()` | Delivery |
| `Courier` | Представляет курьера, его доступность и рейтинг. | `courierId` (int), `name` (std::string), `phoneNumber` (std::string), `available` (bool), `completedDeliveries` (int), `rating` (double), `ratingCount` (int) | `id()`, `name()`, `phoneNumber()`, `isAvailable()`, `assignDelivery()`, `completeDelivery()`, `setAvailable()`, `addRating()`, `completedDeliveries()`, `rating()` | — (Delivery хранит courierId) |
| `Delivery` | Связывает заказ и клиента с адресом и курьером. | `deliveryId` (int), `orderId` (int), `customerId` (int), `address` (DeliveryAddress), `courierId` (int), `status` (DeliveryStatus), `fee` (double), `createdAt` (std::string) | `id()`, `status()`, `courierId()`, `fee()`, `createdAt()`, `address()`, `assignCourier()`, `markPickedUp()`, `complete()`, `cancel()`, `isActive()`, `belongsToCustomer()` | Order, Customer, DeliveryAddress, Courier |

### Statistics

| Класс | Назначение | Поля | Методы | Ассоциации |
|---|---|---|---|---|
| `SalesReport` | Рассчитывает выручку, скидки, возвраты и средний чек за период. | `periodStart` (std::string), `periodEnd` (std::string), `orderCount` (int), `grossRevenue` (double), `discounts` (double), `refunds` (double) | `recordSale()`, `recordRefund()`, `netRevenue()`, `averageCheck()`, `orderCount()`, `hasSales()`, `periodStart()`, `periodEnd()` | Bill, Refund |
| `InventoryReport` | Формирует сводку по остаткам и стоимости склада. | `generatedAt` (std::string), `itemCount` (int), `lowStockCount` (int), `totalValue` (double), `totalQuantity` (double) | `updateFrom()`, `hasLowStock()`, `averageItemValue()`, `itemCount()`, `lowStockCount()`, `totalValue()`, `totalQuantity()`, `generatedAt()` | Inventory |
| `MenuPerformanceReport` | Считает количество продаж и популярность позиций меню. | `period` (std::string), `soldItems` (std::map<int, int>), `totalItemsSold` (int), `revenue` (double), `unavailableEvents` (int) | `recordSale()`, `recordUnavailableEvent()`, `quantitySold()`, `popularItemId()`, `revenue()`, `totalItemsSold()`, `unavailableEvents()`, `period()` | MenuItem |
| `StaffPerformanceReport` | Собирает показатели выполненных задач и рабочего времени. | `period` (std::string), `completedTasks` (std::map<int, int>), `workedMinutes` (std::map<int, int>), `lateArrivals` (int), `absences` (int) | `recordTask()`, `recordWorkedTime()`, `recordLateArrival()`, `recordAbsence()`, `tasksCompleted()`, `workedMinutes()`, `punctualityIncidents()`, `hasRecords()`, `period()` | Chef, Employee |
| `RestaurantStatistics` | Сводит общие показатели заказов, клиентов, выручки и доставок. | `totalOrders` (int), `completedOrders` (int), `cancelledOrders` (int), `totalCustomers` (int), `completedDeliveries` (int), `totalRevenue` (double), `totalRefunds` (double) | `recordOrder()`, `recordCustomer()`, `recordPayment()`, `recordRefund()`, `recordDelivery()`, `netRevenue()`, `averageOrderValue()`, `cancellationRate()`, `totalOrders()`, `completedOrders()`, `completedDeliveries()` | Order, Customer, Payment, Refund, Delivery |

## Исключения

Все специализированные исключения наследуются от `RestaurantException`. Они не добавляют собственных полей и специализированных методов: используют унаследованное сообщение `std::runtime_error`, передаваемое при создании объекта.

| Исключение | Когда возникает |
|---|---|
| `RestaurantException` | Базовое исключение системы. |
| `RestaurantClosedException` | Кухня или ресторан не принимает операции. |
| `TableNotFoundException` | Столик не найден. |
| `TableUnavailableException` | Столик занят или недоступен. |
| `InvalidReservationException` | Некорректное или недопустимое бронирование. |
| `InvalidOrderException` | Недопустимые данные или операция с заказом. |
| `DishNotFoundException` | Блюдо или позиция заказа не найдена. |
| `InsufficientStockException` | На складе недостаточно продукта. |
| `IngredientUnavailableException` | Ингредиент отсутствует или не найден. |
| `PaymentFailedException` | Платёж или возврат не может быть выполнен. |
| `EmployeeNotAvailableException` | Сотрудник недоступен для задачи. |
| `ShiftConflictException` | Смены пересекаются или нарушают интервал отдыха. |
| `DeliveryUnavailableException` | Невозможно продолжить операцию доставки. |
| `SupplierNotFoundException` | Поставщик недоступен для закупки. |

## Консольный интерфейс

CLI позволяет показать меню, зарегистрировать клиента, забронировать столик, создать заказ по подтверждённому бронированию или на вынос, оплатить последний заказ наличными или банковской картой, посмотреть состояние столиков, отметить уборку столика и вывести сводную статистику. После успешной оплаты бронирование завершается, а столик ожидает уборки; после уборки он снова становится доступным. Программа запускается из консоли и не требует IDE.

## Тестирование

`tests.cpp` содержит 235 проверок поведения предметной модели, включая корректные операции, граничные значения, связь заказа с бронированием и переходы столика после успешной и неуспешной оплаты, а также ожидаемые исключения. Тесты запускаются из консоли без стороннего тестового фреймворка.