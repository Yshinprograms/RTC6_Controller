#pragma once

class InterfaceCommunicator {
public:
	virtual ~InterfaceCommunicator() = default;
	virtual bool connectAndSetupBoard() = 0;
	virtual bool isSuccessfullySetup() const = 0;
};