#pragma once

// Cover — Sara's unit of measurement.
// She counts covers, not orders. A cover is one person seated at a table.
// This is the fundamental metric for floor operations: covers per night,
// covers per table turn, covers per service.
//
// There is no Cover class — cover_count lives on Seating.
// This header exists as a conceptual marker in the ubiquitous language.

namespace floor_ctx {
namespace covers {
    // Sara counts covers. Everything on the floor revolves around this number.
} // namespace covers
} // namespace floor_ctx
