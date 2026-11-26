#pragma once

#include <stdexcept>
#include <string>

namespace restaurant {

class RestaurantException : public std::runtime_error {
public:
    explicit RestaurantException(const std::string& message)
        : std::runtime_error(message) {}
};

class RestaurantClosedException : public RestaurantException {
public:
    using RestaurantException::RestaurantException;
};

class TableNotFoundException : public RestaurantException {
public:
    using RestaurantException::RestaurantException;
};

class TableUnavailableException : public RestaurantException {
public:
    using RestaurantException::RestaurantException;
};

class InvalidReservationException : public RestaurantException {
public:
    using RestaurantException::RestaurantException;
};

class InvalidOrderException : public RestaurantException {
public:
    using RestaurantException::RestaurantException;
};

class DishNotFoundException : public RestaurantException {
public:
    using RestaurantException::RestaurantException;
};

class InsufficientStockException : public RestaurantException {
public:
    using RestaurantException::RestaurantException;
};

class IngredientUnavailableException : public RestaurantException {
public:
    using RestaurantException::RestaurantException;
};

class PaymentFailedException : public RestaurantException {
public:
    using RestaurantException::RestaurantException;
};

class EmployeeNotAvailableException : public RestaurantException {
public:
    using RestaurantException::RestaurantException;
};

class ShiftConflictException : public RestaurantException {
public:
    using RestaurantException::RestaurantException;
};

class DeliveryUnavailableException : public RestaurantException {
public:
    using RestaurantException::RestaurantException;
};

class SupplierNotFoundException : public RestaurantException {
public:
    using RestaurantException::RestaurantException;
};

} // namespace restaurant