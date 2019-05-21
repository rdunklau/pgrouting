/*PGR-GNU*****************************************************************

Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

Copyright (c) 2017 Maoguang Wang
Mail: xjtumg1007@gmail.com

------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

********************************************************************PGR-GNU*/

#ifndef INCLUDE_COMPONENTS_PGR_COMPONENTS_HPP_
#define INCLUDE_COMPONENTS_PGR_COMPONENTS_HPP_
#pragma once

#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/biconnected_components.hpp>

#include <vector>
#include <map>
#include <utility>
#include <algorithm>

#include "cpp_common/pgr_base_graph.hpp"
#include "components/componentsResult.h"

namespace pgrouting {
namespace algorithms {

/**
 * works for undirected graph
 **/
template < class G >
std::vector<pgr_components_rt>
connectedComponents(G &graph) {
    size_t totalNodes = num_vertices(graph.graph);

    // perform the algorithm
    std::vector< int > components(totalNodes);
    auto num_comps = boost::connected_components(graph.graph, &components[0]);

    // get the results
    std::vector< std::vector< int64_t > > results;
    results.resize(num_comps);
    for (size_t i = 0; i < totalNodes; i++)
        results[components[i]].push_back(graph[i].id);

    return detail::componentsResult(results);
}

//! Strongly Connected Components Vertex Version
/**
 * works for directed graph
 **/
template < class G >
std::vector<pgr_components_rt>
strongComponents(
        G &graph) {
    size_t totalNodes = num_vertices(graph.graph);

    // perform the algorithm
    std::vector< int > components(totalNodes);
    int num_comps = boost::strong_components(graph.graph,
            boost::make_iterator_property_map(components.begin(),
                                              get(boost::vertex_index,
                                                  graph.graph)));

    // get the results
    std::vector< std::vector< int64_t > > results;
    results.resize(num_comps);
    for (size_t i = 0; i < totalNodes; i++)
        results[components[i]].push_back(graph[i].id);

    return detail::componentsResult(results);
}

//! Biconnected Components (for undirected)
template < class G >
std::vector<pgr_components_rt>
biconnectedComponents(
        G &graph) {
     using E =  typename G::E;
     using E_i = typename G::E_i;
    // perform the algorithm
    struct order_edges {
        bool operator() (const E &left, const E &right) const {
            return left.get_property() < right.get_property();
        }
    };
    typedef std::map< E, size_t > edge_map;
    edge_map bicmp_map;

    boost::associative_property_map< edge_map > bimap(bicmp_map);
    size_t num_comps = biconnected_components(graph.graph, bimap);

    // get the results
    E_i ei, ei_end;
    std::vector< std::vector< int64_t > > components(num_comps);
    for (boost::tie(ei, ei_end) = edges(graph.graph); ei != ei_end; ei++)
        components[bimap[*ei]].push_back(graph[*ei].id);

    return detail::componentsResult(components);
}

template < class G >
std::vector<pgr_components_rt>
articulationPoints(
        G &graph) {
    // perform the algorithm
    std::vector <size_t> art_points;
    boost::articulation_points(graph.graph, std::back_inserter(art_points));

    // get the results
    std::vector <pgr_components_rt> results;
    size_t totalArtp = art_points.size();
    results.resize(totalArtp);
    for (size_t i = 0; i < totalArtp; i++)
        results[i].identifier = graph[art_points[i]].id;

    // sort identifier
    std::sort(results.begin(), results.end(),
            [](const pgr_components_rt &left, const pgr_components_rt &right) {
            return left.identifier < right.identifier; });

    return results;
}

//! Bridges
template < class G >
std::vector<pgr_components_rt>
bridges(
        G &graph) {
    using E =  typename G::E;
    using E_i = typename G::E_i;
    size_t totalNodes = num_vertices(graph.graph);
    std::vector< int > tmp_comp(totalNodes);
    std::vector <pgr_components_rt> results;
    int ini_comps = boost::connected_components(graph.graph, &tmp_comp[0]);

    // perform the algorithm
    E_i ei, ei_end;
    std::vector< std::pair<E, int64_t> > stored_edges;
    for (boost::tie(ei, ei_end) = edges(graph.graph); ei != ei_end; ++ei) {
        stored_edges.push_back(std::make_pair(*ei, graph[*ei].id));
    }

    for (const auto pair_edge : stored_edges) {
        E edge = pair_edge.first;

        boost::remove_edge(edge, graph.graph);

        int now_comps = boost::connected_components(graph.graph, &tmp_comp[0]);
        if (now_comps > ini_comps) {
            pgr_components_rt temp;
            temp.identifier = pair_edge.second;
            results.push_back(temp);
        }

        boost::add_edge(boost::source(edge, graph.graph),
                        boost::target(edge, graph.graph),
                        graph.graph);
    }

    // sort identifier
    std::sort(results.begin(), results.end(),
            [](const pgr_components_rt &left, const pgr_components_rt &right) {
            return left.identifier < right.identifier; });

    return results;
}

}  // namespace algorithms
}  // namespace pgrouting

#endif  // INCLUDE_COMPONENTS_PGR_COMPONENTS_HPP_
