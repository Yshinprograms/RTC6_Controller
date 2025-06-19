#pragma once

class InterfacePrintController {
public:
    virtual ~InterfacePrintController() = default;

    /**
     * @brief Executes the entire print job from start to finish.
     */
    virtual void run() = 0;
};