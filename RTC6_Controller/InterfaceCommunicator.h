#pragma once

class InterfaceCommunicator {
public:
	virtual ~InterfaceCommunicator() = default;

	virtual bool isSuccessfullySetup() const = 0;
};