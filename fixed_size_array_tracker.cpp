#include "fixed_size_array_tracker.hpp"
#include <vector>
#include <string>
#include <sstream>

FixedSizeArrayTracker::FixedSizeArrayTracker(unsigned int size, bool logging_enabled)
    : size(size), logging_enabled(logging_enabled) {}

// returns a normalized value in [0, 1]
double FixedSizeArrayTracker::get_usage_percentage() const {
    unsigned int used_space = 0;
    for (const auto &[id, range] : metadata) {
        used_space += range.second;
    }
    return (static_cast<double>(used_space) / size);
}

// returns the index to the space with the contiguous space
std::optional<unsigned int> FixedSizeArrayTracker::find_contiguous_space(unsigned int length) {
    unsigned int last_end = 0;

    // iterate over intervals, and check if the gap size between intevals is large enough to store it
    for (const auto &[start, end] : occupied_intervals) {
        if (start - last_end >= length) {
            return last_end; // Found space before this interval
        }
        last_end = end;
    }

    // check if there's enough space after the last interval
    if (size - last_end >= length) {
        return last_end;
    }

    return std::nullopt;
}

bool FixedSizeArrayTracker::add_metadata(int id, unsigned int start, unsigned int length) {
    GlobalLogSection _("add_metadata", logging_enabled);

    if (metadata.count(id)) {
        global_logger->info("ID '" + std::to_string(id) + "' already exists. Use a unique ID.");
        return false;
    }

    if (start + length > size) {
        global_logger->info("Error: Metadata exceeds array bounds.");
        return false;
    }

    // TODO: could be more efficient
    for (const auto &interval : occupied_intervals) {
        if (!(start + length <= interval.first || start >= interval.second)) {
            global_logger->info("Error: Metadata collides with an existing interval.");
            return false;
        }
    }

    // Add metadata and update occupied intervals
    metadata[id] = {start, length};
    occupied_intervals.insert({start, start + length});

    global_logger->info("Added metadata: ID=" + std::to_string(id) + ", start=" + std::to_string(start) +
                        ", length=" + std::to_string(length));

    return true;
}

void FixedSizeArrayTracker::remove_metadata(int id) {
    GlobalLogSection _("remove_metadata", logging_enabled);
    auto it = metadata.find(id);
    if (it != metadata.end()) {
        // Remove metadata and update intervals
        auto &[start, length] = it->second;
        occupied_intervals.erase({start, start + length});
        metadata.erase(it);

        global_logger->info("Removed metadata for ID=" + std::to_string(id));
    } else {
        global_logger->info("ID '" + std::to_string(id) + "' not found.");
    }
}

std::optional<std::pair<unsigned int, unsigned int>> FixedSizeArrayTracker::get_metadata(int id) const {
    auto it = metadata.find(id);
    if (it != metadata.end()) {
        return it->second;
    }
    return std::nullopt;
}

void FixedSizeArrayTracker::compact() {
    GlobalLogSection _("compact", logging_enabled);
    unsigned int current_index = 0;
    std::unordered_map<int, std::pair<unsigned int, unsigned int>> new_metadata;

    // reassign contiguously at the start to eliminate gaps
    for (const auto &[id, range] : metadata) {
        unsigned int length = range.second;
        new_metadata[id] = {current_index, length};
        current_index += length;
    }

    metadata = std::move(new_metadata); // move the new metadata
    occupied_intervals.clear();         // clear the current intervals
    for (const auto &[id, range] : metadata) {
        unsigned int start = range.first;
        unsigned int length = range.second;
        occupied_intervals.insert({start, start + length});
    }

    global_logger->info("Compacted metadata.");
}

const std::unordered_map<int, std::pair<unsigned int, unsigned int>> &FixedSizeArrayTracker::get_all_metadata() const {
    return metadata;
}

std::string FixedSizeArrayTracker::to_string() {
    std::ostringstream os;

    os << "Metadata: {";
    for (const auto &[id, range] : get_all_metadata()) {
        os << id << ": (start=" << range.first << ", length=" << range.second << "), ";
    }
    os << "}\n";

    std::string representation(size, ' ');
    for (const auto &[id, range] : get_all_metadata()) {
        if (range.second > 0) {
            representation[range.first] = '0' + (id % 10); // Display only last digit of ID
            std::fill(representation.begin() + range.first + 1, representation.begin() + range.first + range.second,
                      '-');
        }
    }

    os << representation << "\n";

    // Index markers
    for (unsigned int i = 0; i < size; ++i) {
        os << (i % 10);
    }
    os << "\n";

    // Print numeric indices below (aligned roughly)
    for (unsigned int i = 0; i < size; ++i) {
        if (i % 10 == 0) {
            os << std::to_string(i);
            // Fill spaces until next marker to keep alignment
            size_t next = std::min<size_t>(size, i + 10);
            for (size_t j = i + std::to_string(i).size(); j < next; ++j)
                os << " ";
        }
    }
    os << "\n";

    return os.str();
}

std::ostream &operator<<(std::ostream &os, const FixedSizeArrayTracker &tracker) {
    os << "Metadata: {";
    for (const auto &[id, range] : tracker.get_all_metadata()) {
        os << id << ": (start=" << range.first << ", length=" << range.second << "), ";
    }
    os << "}\n";

    std::string representation(tracker.size, ' ');
    for (const auto &[id, range] : tracker.get_all_metadata()) {
        if (range.second > 0) {
            representation[range.first] = '0' + (id % 10); // Display only the last digit of ID
            std::fill(representation.begin() + range.first + 1, representation.begin() + range.first + range.second,
                      '-');
        }
    }

    os << representation << "\n";
    for (unsigned int i = 0; i < tracker.size; ++i) {
        os << (i % 10); // Keep index alignment simple
    }
    os << "\n";
    for (unsigned int i = 0; i < tracker.size; ++i) {
        os << ((i % 10 == 0) ? std::to_string(i) : " ");
    }
    os << "\n";

    return os;
}
