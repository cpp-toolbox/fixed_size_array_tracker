#ifndef FIXED_SIZE_ARRAY_TRACKER_HPP
#define FIXED_SIZE_ARRAY_TRACKER_HPP

#include <unordered_map>
#include <set>
#include <optional>
#include <iostream>

#include "sbpt_generated_includes.hpp"

/**
 * @class FixedSizeArrayTracker
 * @brief Tracks and manages the allocation of contiguous regions within a fixed-size array.
 *
 * This class helps track metadata about segments (identified by integer IDs) that occupy
 * parts of a fixed-size array. It can find free contiguous regions, allocate or remove
 * metadata entries, and provide visual or textual representations of the current layout.
 */
class FixedSizeArrayTracker {
  public:
    /**
     * @brief Constructs a FixedSizeArrayTracker with a specified array size.
     * @param size The total size of the array to track.
     * @param logging_enabled If true, enables debug logging to standard output.
     */
    FixedSizeArrayTracker(unsigned int size, LogSection::LogMode log_mode = LogSection::LogMode::disable);

    /**
     * @brief Logs a message to the console if logging is enabled.
     * @param message The message to log.
     */
    void log(const std::string &message) const;

    /**
     * @brief Calculates how much of the tracked array is currently occupied.
     * @return A double representing the percentage of the array that is used (0.0–100.0).
     */
    double get_usage_percentage() const;

    /**
     * @brief Finds the first contiguous region of free space large enough to fit the requested length.
     * @param length The number of contiguous elements required.
     * @return An optional starting index for the free region, or std::nullopt if none found.
     */
    std::optional<unsigned int> find_contiguous_space(unsigned int length);

    /**
     * @brief Adds a new metadata entry corresponding to an allocated region.
     * @param id The identifier for the metadata entry.
     * @param start The starting index of the region.
     * @param length The length of the region.
     * @return True if the metadata was added successfully; false if the region overlaps or is invalid.
     */
    bool add_metadata(int id, unsigned int start, unsigned int length);

    /**
     * @brief Removes a metadata entry and frees its associated region.
     * @param id The identifier of the metadata entry to remove.
     */
    void remove_metadata(int id);

    /**
     * @brief Retrieves the metadata associated with a given ID.
     * @param id The identifier to query.
     * @return An optional pair {start, length} describing the region, or std::nullopt if not found.
     */
    std::optional<std::pair<unsigned int, unsigned int>> get_metadata(int id) const;

    /**
     * @brief Rearranges metadata to eliminate gaps between allocated regions.
     *
     * After compaction, all allocated regions are moved to the lowest available
     * positions in the array while preserving their relative order.
     */
    void compact();

    /**
     * @brief Produces a human-readable string representation of the current tracker state.
     * @return A formatted string containing metadata and layout visualization.
     */
    std::string to_string();

    /**
     * @brief Retrieves all current metadata entries.
     * @return A const reference to the internal metadata map (id → {start, length}).
     */
    const std::unordered_map<int, std::pair<unsigned int, unsigned int>> &get_all_metadata() const;

    /**
     * @brief Overloads the stream insertion operator to print the tracker’s current state.
     * @param os The output stream to write to.
     * @param tracker The tracker instance to print.
     * @return A reference to the output stream.
     */
    friend std::ostream &operator<<(std::ostream &os, const FixedSizeArrayTracker &tracker);

  private:
    /// the total size of the tracked array.
    unsigned int size;

    LogSection::LogMode log_mode = LogSection::LogMode::disable;

    /// maps metadata ids to their associated regions (start index and length).
    std::unordered_map<int, std::pair<unsigned int, unsigned int>> metadata;

    /// stores occupied regions as sorted intervals (start, end).
    std::set<std::pair<unsigned int, unsigned int>> occupied_intervals;
};

#endif // FIXED_SIZE_ARRAY_TRACKER_HPP
