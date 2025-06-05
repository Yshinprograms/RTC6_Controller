// Implementations/ListCommandBuilder.cpp
#include "ListCommandBuilder.h"

// Constructor: Initializes with the low-level RTC6 driver.
ListCommandBuilder::ListCommandBuilder(IRTCAPIDriver* driver)
    : driver_(driver) {
    if (driver_ == nullptr) {
        throw std::invalid_argument("ListCommandBuilder: IRTCAPIDriver pointer cannot be null.");
    }
}

// --- Implementations of IListCommandBuilder methods ---

// List Control
void ListCommandBuilder::SetStartList(UINT listNumber) {
    driver_->set_start_list(listNumber);
}

void ListCommandBuilder::SetEndOfList() {
    driver_->set_end_of_list();
}

void ListCommandBuilder::ExecuteList(UINT listNumber) {
    driver_->execute_list(listNumber);
}

// Basic Motion Commands
void ListCommandBuilder::JumpAbsolute(LONG x, LONG y) {
    driver_->jump_abs(x, y);
}

void ListCommandBuilder::MarkAbsolute(LONG x, LONG y) {
    driver_->mark_abs(x, y);
}

// Micro-vector Command
void ListCommandBuilder::AddMicroVector(LONG x, LONG y, LONG laserOnBits, LONG laserOffBits) {
    driver_->micro_vector_abs(x, y, laserOnBits, laserOffBits);
}

// Dynamic Parameter Control (List-based)
void ListCommandBuilder::SetCurrentListLaserPower(UINT powerValue) {
    driver_->set_laser_power_list(powerValue);
}

void ListCommandBuilder::SetCurrentListMarkSpeed(double speedValue) {
    driver_->set_mark_speed_list(speedValue);
}

void ListCommandBuilder::SetCurrentListZAxisHeight(LONG zAxisValue) {
    driver_->set_defocus_list(zAxisValue);
}