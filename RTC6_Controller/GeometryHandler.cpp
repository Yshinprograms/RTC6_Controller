#include "GeometryHandler.h"
#include <iostream>
#include <cmath>

GeometryHandler::GeometryHandler(InterfaceListHandler& listHandler)
	: m_listHandler(listHandler) {
	std::cout << "[GeometryHandler] Instance created." << std::endl;
}

GeometryHandler::~GeometryHandler() = default;

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