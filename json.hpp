#ifndef JSON_HPP
#define JSON_HPP

#include "shelter.hpp"

// Formatting helpers for communication with the browser, not business rules.
inline std::string jsonString(const std::string& text) {
    std::ostringstream out;
    out << '"';
    for (unsigned char ch : text) {
        if (ch == '"') out << "\\\"";
        else if (ch == '\\') out << "\\\\";
        else if (ch < 32) out << "\\u" << std::hex << std::setw(4) << std::setfill('0') << int(ch) << std::dec;
        else out << ch;
    }
    out << '"';
    return out.str();
}

inline std::string serialize(const Shelter& shelter) {
    std::ostringstream out;
    out << "{\"capacity\":" << shelter.getCapacity()
        << ",\"occupancy\":" << shelter.getOccupancy() << ",\"resources\":[";
    bool first = true;
    for (const auto& resource : shelter.getResources()) {
        if (!first) out << ',';
        first = false;
        const IExpirable* expirable = dynamic_cast<const IExpirable*>(resource.get());
        out << "{\"id\":" << resource->getId()
            << ",\"name\":" << jsonString(resource->getName())
            << ",\"type\":" << jsonString(resource->getType())
            << ",\"unit\":" << jsonString(resource->getUnit())
            << ",\"quantity\":" << resource->getQuantity()
            << ",\"usable\":" << (resource->isUsable() ? "true" : "false")
            << ",\"expiry\":" << jsonString(expirable ? expirable->getExpiry() : "") << '}';
    }
    out << "],\"residents\":[";
    first = true;
    for (const Resident& person : shelter.getResidents()) {
        if (!first) out << ',';
        first = false;
        out << "{\"id\":" << person.getId() << ",\"name\":" << jsonString(person.getName())
            << ",\"members\":" << person.getMembers() << '}';
    }
    out << "],\"allocations\":[";
    first = true;
    for (const Allocation& record : shelter.getAllocations()) {
        if (!first) out << ',';
        first = false;
        out << "{\"residentId\":" << record.residentId << ",\"resourceId\":" << record.resourceId
            << ",\"quantity\":" << record.quantity << '}';
    }
    out << "],\"restocks\":[";
    first = true;
    for (const Restock& record : shelter.getRestocks()) {
        if (!first) out << ',';
        first = false;
        out << "{\"resourceId\":" << record.resourceId << ",\"quantity\":" << record.quantity
            << ",\"donor\":" << jsonString(record.donor) << '}';
    }
    out << "]}";
    return out.str();
}
#endif
