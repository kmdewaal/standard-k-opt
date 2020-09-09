#include "DistanceTable.h"
#include "Optimizer.h"
#include "PointSequence.h"
#include "Segment.h"
#include "fileio/PointSet.h"
#include "fileio/Tour.h"
#include "primitives.h"
#include "verify.h"

#include <iostream>
#include <vector>
#include <iomanip>
#include <chrono>

int main(int argc, char** argv)
{
    // Read points.
    if (argc < 2)
    {
        std::cout << "Arguments: point_set_file_path optional_tour_file_path" << std::endl;
        return 0;
    }
    fileio::PointSet point_set(argv[1]);
    // Initial tour.
    std::vector<primitives::point_id_t> tour;
    if (argc > 2)
    {
        fileio::Tour initial_tour(argv[2]);
        tour = initial_tour.point_ids();
    }
    else
    {
        for (primitives::point_id_t i{0}; i < point_set.count(); ++i)
        {
            tour.push_back(i);
        }
    }
    // Initialize distance table.
    PointSequence point_sequence(tour);
    DistanceTable dt(point_set.x(), point_set.y());
    // Initialize segments.
    const auto& next = point_sequence.next();
    Segment::Container segments;
    for (auto id : tour)
    {
        auto length = dt.lookup_length(id, next[id]);
        segments.insert({id, next[id], length});
    }

    // KdW print results
    // std::cout << "Old segments:" << std::endl;
    // for (auto & segref : segments)
    // {
    //     std::cout << segref << std::endl;
    // }

    // Hill climbing optimization loop.
    Optimizer optimizer(dt, point_sequence.sequence_ids());
    const auto n = point_set.x().size();
    std::cout << "Expecting " << (n - 1) * (n - 2) / 2 - 1 << " checks per 2-opt best-improvement iteration.\n";
    std::cout << "Starting hill-climbing optimization:\n";
    auto prev_length = verify::tour_length(segments, dt);
    std::cout << "Initial tour length: " << prev_length << std::endl;
    primitives::length_t cycle_improvement{0};
    int iteration{1};

    auto t0 = std::chrono::high_resolution_clock::now();
    do
    {
        cycle_improvement = 0;
        // for (size_t k{2}; k < 5; ++k)
        for (size_t k{3}; k < 4; ++k)       // KdW test
        {
            optimizer.k(k);
            primitives::length_t improvement{0};
            do
            {
                optimizer.find_best(segments);
                point_sequence.new_tour(segments, optimizer.best().segments, optimizer.best().new_segments);
                constexpr bool debug_mode{false};
                if (debug_mode)
                {
                    std::cout << optimizer << std::endl;
                    if (verify::valid_cycle(segments))
                    {
                        std::cout << "Tour is still a valid cycle." << std::endl;
                    }
                    else
                    {
                        std::cout << "ERROR: tour has become invalid!" << std::endl;
                        cycle_improvement = -1;
                        break;
                    }
                }
                constexpr int print_period{1};
                if (iteration % print_period == 0)
                {
                    auto current_length = verify::tour_length(segments, dt);
                    improvement = prev_length - current_length;
                    cycle_improvement += improvement;
                    std::cout << "Iteration " << iteration
                        << " final tour length: " << current_length
                        << " (improvement since last update: " << improvement << ")\n";
                    prev_length = current_length;
                }
                ++iteration;
            } while (improvement > 0);
            std::cout << k << "-optimal solution found." << std::endl;
            auto t1 = std::chrono::high_resolution_clock::now();
            auto td = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
            std::cout << "Execution time since start: " << td << " microseconds" << std::endl;
        }
    } while (cycle_improvement > 0);
    if (cycle_improvement == 0)
    {
        std::cout << "Local optimum reached." << std::endl;
    }
    else
    {
        std::cout << "Hill-climbing optimization prematurely stopped." << std::endl;
    }

    // KdW print results

    // std::cout << "Original route:" << std::endl;
    // for (primitives::point_id_t i{0}; i < point_set.count(); ++i)
    // {
    //     std::cout << i << " " << tour[i] << "  " << point_set.x()[i] << "  " << point_set.y()[i] << std::endl;
    // }

    // std::cout << "New segments:" << std::endl;
    // for (auto & segref : segments)
    // {
    //     std::cout << segref << std::endl;
    // }

    std::vector<int> np;
    np.resize(point_set.count());
    for (auto & segref : segments)
    {
        // std::cout << "segref.a:" << segref.a << "  segref.b:" << segref.b << std::endl;
        np[segref.a] = segref.b;
    }

    // std::cout << "Size of np:" << np.size() << std::endl;

    // Complete route, sorted
    // std::cout << "Sorted array: i, np[i], x[np[i]], y[np[i]]" << std::endl;
    // for (size_t i = 0; i< np.size(); i++)
    // {
    //     std::cout << std::setw(5) << i << "  ";
    //     std::cout << std::setw(5) << np[i] << "  ";
    //     std::cout << std::setw(5) << point_set.x()[np[i]] << "  ";
    //     std::cout << std::setw(5) << point_set.y()[np[i]] << std::endl;
    // }

    return 0;
}
