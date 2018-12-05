#include "PointSequence.h"

PointSequence::PointSequence(const std::vector<primitives::point_id_t>& sequence)
{
    m_adjacents.resize(sequence.size());
    for (auto& a : m_adjacents)
    {
        a = {primitives::InvalidPoint, primitives::InvalidPoint};
    }
    auto prev = sequence.back();
    for (auto p : sequence)
    {
        create_adjacency(p, prev);
        prev = p;
    }
    m_next.resize(sequence.size());
    update_next();
}

void PointSequence::update_next()
{
    primitives::point_id_t current{0};
    m_next[current] = m_adjacents[current].front();
    do
    {
        auto prev = current;
        current = m_next[current];
        m_next[current] = get_other(current, prev);
    } while (current != 0); // cycle condition.
}

void PointSequence::reorder(const std::vector<Segment>& old_segments, const std::vector<Segment>& new_segments)
{
    // break old segments.
    for (const auto& s : old_segments)
    {
        break_adjacency(s.a, s.b);
    }
    // form new segments.
    for (const auto& s : new_segments)
    {
        create_adjacency(s.a, s.b);
    }
}

primitives::point_id_t PointSequence::get_other(primitives::point_id_t point, primitives::point_id_t adjacent) const
{
    const auto& a = m_adjacents[point];
    if (a.front() == adjacent)
    {
        return a.back();
    }
    else
    {
        return a.front();
    }
}
void PointSequence::create_adjacency(primitives::point_id_t point1, primitives::point_id_t point2)
{
    fill_adjacent(point1, point2);
    fill_adjacent(point2, point1);
}
void PointSequence::fill_adjacent(primitives::point_id_t point, primitives::point_id_t new_adjacent)
{
    if (m_adjacents[point].front() == primitives::InvalidPoint)
    {
        m_adjacents[point].front() = new_adjacent;
    }
    else if (m_adjacents[point].back() == primitives::InvalidPoint)
    {
        m_adjacents[point].back() = new_adjacent;
    }
}
void PointSequence::break_adjacency(primitives::point_id_t point1, primitives::point_id_t point2)
{
    vacate_adjacent_slot(point1, point2, 0);
    vacate_adjacent_slot(point1, point2, 1);
    vacate_adjacent_slot(point2, point1, 0);
    vacate_adjacent_slot(point2, point1, 1);
}

void PointSequence::vacate_adjacent_slot(primitives::point_id_t point, primitives::point_id_t adjacent, int slot)
{
    if (m_adjacents[point][slot] == adjacent)
    {
        m_adjacents[point][slot] = primitives::InvalidPoint;
    }
}

void PointSequence::align(Segment::Container& segments) const
{
    std::vector<Segment> reverse;
    for (auto& s : segments)
    {
        if (s.a == m_next[s.b])
        {
            reverse.push_back(s);
        }
    }
    for (const auto& s : reverse)
    {
        segments.erase(s);
    }
    for (auto& s : reverse)
    {
        std::swap(s.a, s.b);
        segments.insert(s);
    }
}

void PointSequence::new_tour(Segment::Container& segments
    , const std::vector<Segment>& old_segments, const std::vector<Segment>& new_segments)
{
    for (const auto& s : old_segments)
    {
        auto erased = segments.erase(s);
        if (erased != 1)
        {
            std::cout << "ERROR: " << erased << " segments were erased (should only be 1)." << std::endl;
        }
    }
    for (const auto& s : new_segments)
    {
        auto pair = segments.insert(s);
        if (not pair.second)
        {
            std::cout << "ERROR: could not insert segment!" << std::endl;
        }
    }
    reorder(old_segments, new_segments);
    update_next();
    align(segments);
}

