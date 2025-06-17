#include "GeometryHandler.h"
#include <iostream>
#include <cmath>

GeometryHandler::GeometryHandler(InterfaceListHandler& listHandler)
	: m_listHandler(listHandler) {
	std::cout << "[GeometryHandler] Instance created." << std::endl;
}

GeometryHandler::~GeometryHandler() = default;


/**
 * @brief Translates a single OVF VectorBlock into a sequence of RTC6 list commands.
 *
 * This function acts as the bridge between the OVF data model and the hardware control
 * model. It is the "specialist" that knows how to interpret a piece of geometry and
 * its associated parameters, and then delegate the correct API calls to the ListHandler.
 *
 * The process is a two-step sequence:
 * 1.  **Set Parameters:** It first sets the RTC6 list parameters (speed, power, focus)
 *     that will apply to the entire block.
 * 2.  **Generate Geometry:** It then uses a switch statement to handle the specific
 *     type of geometry in the block (e.g., a continuous line, a series of hatches).
 *     Based on the type, it iterates through the points and calls the appropriate
 *     jump and mark commands on the ListHandler.
 *
 * Tracks:
 * - Laser speed in mm/s
 * - Focus offset in mm (converted to bits)
 * - Laser power in percentage (converted to DAC value)
 * 
 * @param block The OVF VectorBlock to process, containing the geometry data. Passed
 *              by const reference for high efficiency.
 * @param params The OVF MarkingParams for this block, containing the laser settings.
 *               Passed by const reference.
 */
void GeometryHandler::processVectorBlock(
	const open_vector_format::VectorBlock& block,
	const open_vector_format::MarkingParams& params)
{
	// 1. Set the process parameters for this specific block
	m_listHandler.addSetMarkSpeed(params.laser_speed_in_mm_per_s());
	m_listHandler.addSetFocusOffset(mmToBits(params.laser_focus_shift_in_mm()));

	double powerPercent = (params.laser_power_in_w() / MAX_LASER_POWER_W) * 100.0;
	m_listHandler.addSetLaserPower(1, powerToDAC(powerPercent));

	// 2. Process the geometry based on its type
	switch (block.vector_data_case()) {
	case open_vector_format::VectorBlock::kLineSequence: {
		const auto& points = block.line_sequence().points();
		if (points.size() < 2) return;

		// FIX: Use square brackets [] for element access
		m_listHandler.addJumpAbsolute(mmToBits(points[0]), mmToBits(points[1]));
		for (int i = 2; i < points.size(); i += 2) {
			m_listHandler.addMarkAbsolute(mmToBits(points[i]), mmToBits(points[i + 1]));
		}
		break;
	}

	case open_vector_format::VectorBlock::kHatches: {
		// FIX: Use the correct accessor '_hatches()' and square brackets []
		const auto& points = block._hatches().points();
		for (int i = 0; i < points.size(); i += 4) {
			// FIX: Use square brackets [] for element access
			m_listHandler.addJumpAbsolute(mmToBits(points[i]), mmToBits(points[i + 1]));
			m_listHandler.addMarkAbsolute(mmToBits(points[i + 2]), mmToBits(points[i + 3]));
		}
		break;
	}

	// ToDo: Implement other cases as needed
	// case open_vector_format::VectorBlock::kPointSequence: { ... }
	// case open_vector_format::VectorBlock::kArcs: { ... }

	default:
		// Silently ignore unsupported types for now
		break;
	}
}

// Private helper methods remain the same
int GeometryHandler::mmToBits(double mm) const {
	return static_cast<int>(std::round(mm * BITS_PER_MM));
}

UINT GeometryHandler::powerToDAC(double percent) const {
	if (percent < 0.0) percent = 0.0;
	if (percent > 100.0) percent = 100.0;
	return static_cast<UINT>((percent / 100.0) * 4095.0);
}