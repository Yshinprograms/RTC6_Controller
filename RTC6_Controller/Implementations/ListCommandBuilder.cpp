#include "ListCommandBuilder.h"
#include <stdexcept>

ListCommandBuilder::ListCommandBuilder(IRTCAPIDriver* driver)
    : m_driver(driver) {
    if (m_driver == nullptr) {
        throw std::invalid_argument("IRTCAPIDriver pointer cannot be null");
    }
}

// List control implementation
void ListCommandBuilder::SetStartList(int listNumber) {
    m_driver->SetStartList(listNumber);
}

void ListCommandBuilder::SetEndOfList() {
    m_driver->SetEndOfList();
}

void ListCommandBuilder::ExecuteList(int listNumber) {
    m_driver->ExecuteList(listNumber);
}

int ListCommandBuilder::GetListStatus(int listNumber) const {
    return m_driver->GetListStatus(listNumber);
}

int ListCommandBuilder::GetListPointer() const {
    return m_driver->GetListPointer();
}

// Basic motion commands implementation
void ListCommandBuilder::JumpAbsolute(int x, int y) {
    m_driver->JumpAbsolute(x, y);
}

void ListCommandBuilder::MarkAbsolute(int x, int y) {
    m_driver->MarkAbsolute(x, y);
}

// Micro-vector commands implementation
void ListCommandBuilder::AddMicroVector(int x, int y, int dt) {
    m_driver->AddMicroVector(x, y, dt);
}

void ListCommandBuilder::AddLaserSignalOnList() {
    m_driver->AddLaserSignalOnList();
}

void ListCommandBuilder::AddLaserSignalOffList() {
    m_driver->AddLaserSignalOffList();
}

void ListCommandBuilder::SetCurrentListLaserPower(UINT powerValue) {
    m_driver->SetCurrentListLaserPower(powerValue);
}

void ListCommandBuilder::SetCurrentListMarkSpeed(double speedValue) {
    m_driver->SetCurrentListMarkSpeed(speedValue);
}

void ListCommandBuilder::SetCurrentListZAxisHeight(int zAxisValue) {
    m_driver->SetCurrentListZAxisHeight(zAxisValue);
}
