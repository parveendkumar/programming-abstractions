//
// Trailblazer.h
//
// Implementation of the graph algorithms that comprise the Trailblazer
// assignment.
//
// --------------------------------------------------------------------------
// Assignment: 7 Trailblazer
// Course: CS106B "Programming Abstractions in C++"
// Provider: Stanford University (SCPD)
// Instructor: Keith Schwarz
// Date: Spring 2013
// http://web.stanford.edu/class/archive/cs/cs106b/cs106b.1136/handouts/260%20Assignment%207.pdf
// --------------------------------------------------------------------------
//
// Created by Glenn Streiff on 3/16/2017.
// Copyright © 2017 Glenn Streiff. All rights reserved.
//

#include "Trailblazer.h"
#include "TrailblazerGraphics.h"
#include "TrailblazerTypes.h"
#include "TrailblazerPQueue.h"
#include "TrailblazerCosts.h"

using namespace std;

// Function: shortestPath
// Usage: shortestPath(start, end, world, costFn);
// -----------------------------------------------
// Uses Dijkstra's algorithm to find the least-cost path between two locations.
// The cost of moving from one location to the next is specified by the given
// cost function.
//
// The resulting path is returned as a Vector<Loc> containing the sequence
// of locations to visit from start to end.  If no path is found, this function
// reports and error.

Vector<Loc> shortestPath(Loc start,
                         Loc end,
                         Grid<double>& world,
                         double costFn(Loc from, Loc to, Grid<double>& world)) {
    
    Vector<Loc> path;           // Sequetial path from start to end.
    
    if (start == end) {
        path.add(start);
        return path;
    }
    
    // Track location attributes such as cost-from-start, parent location,
    // queue status, and blacklist status.  (Should probably use a set here
    // to avoid O(n²) memory usage :-| but set was fighting me :-/).
    
    Grid<LocStateT> state(world.nRows, world.nCols);
    initLocState(state, start);
    
    TrailblazerPQueue<Loc> pq;  // Priority queue; cost from start is priority.
    pq.enqueue(start, 0);       // Cost of going from start to start is 0.
    Loc n = start;              // Least cost loc (so far) to consider on path.
    
    while (!pq.isEmpty()) {
        n = pq.dequeueMin();    // Dequeue least cost loc (so far) from start.
        
        setLocEnqueued(state, n, false);
        setLocBlackListed(state, n, true);
        colorCell(world, n, GREEN);

        if (n == end) break;    // Found least cost path to end location. Done!
        
        // Iterate over the locations adjacent to n.
        // Can we improve their 'cost-from-start' value if we pass through n?
        
        Vector<Loc> adjLocs = getAdjLocs(n, world, state);// non-BL'd adjacents
        for (int i = 0; i < adjLocs.size(); i++) {
            
            Loc adj = adjLocs[i];
            double nCost = getLocCostSoFar(state, n);   // node cost
            double eCost = costFn(n, adj, world);       // n-to-adj edge cost
            double costToAdjViaN = nCost + eCost;
            
            // Is this node already in the priority queue?
            // If so, it will have cost info we may want to update.
            
            if (isLocEnqueued(state, adj)){
                double currAdjCost = getLocCostSoFar(state, adj);
                if (costToAdjViaN < currAdjCost) {
                    
                    // It's cheaper to go from start to the adjacent location
                    // if we pass through n first.
                    
                    // Make it more likely we'll dequeue this node on a future
                    // iteration.
                    
                    pq.decreaseKey(adj, costToAdjViaN);
                    setLocCostSoFar(state, adj, costToAdjViaN);
                    setLocParent(state, adj, n);    // Drop cookie crumb.
                }
            } else {
                
                // Wire up new adjacent location it into the graph, so to speak.
                //
                // Its associated cost from the start is just n's plus the edge
                // cost to get from n to the adjacent location.
                
                pq.enqueue(adj, costToAdjViaN);
                setLocEnqueued(state, adj, true);
                setLocCostSoFar(state, adj, costToAdjViaN);
                setLocParent(state, adj, n);
                colorCell(world, adj, YELLOW);
            }
        }
    }
    
    if (n == end) {
        
        // Backtrack from end to start positions, following the parent
        // locations that demarque the path.
        
        for (Loc n = end; n != start; n = state[n.row][n.col].parent) {
            path.insert(0, n);
        }
        path.insert(0, start);
        
    } else {
        error("shortestPath: No path found from start to end.");
    }
    
    return path;
}

// Function: getAdjLocs
// Usage: Vector<Loc> v = getAdjLocs(loc, world, blacklist);
// ---------------------------------------------------------
// Returns a vector of locations adjacent to a given position in a 2D
// grid world.

Vector<Loc> getAdjLocs(const Loc& loc,
                       const Grid<double>& world,
                       Grid<LocStateT>& state) {
    
    Vector<Loc> adjLoc;
    for (int r = loc.row - 1; r <= loc.row + 1; r++) {
        for (int c = loc.col - 1; c <= loc.col + 1; c++) {
            if (world.inBounds(r, c) && !(r == loc.row && c == loc.col)) {
                Loc l = makeLoc(r, c);
                if (state[r][c].blackListed) continue;
                adjLoc.add(l);
            }
        }
    }
    return adjLoc;
}

double getLocCostSoFar(Grid<LocStateT>& state, const Loc& l) {
    return state[l.row][l.col].costSoFar;
}

void initLocState(Grid<LocStateT> state, const Loc& start) {
    for (int r = 0; r < state.nRows; r++) {
        for (int c = 0; c < state.nCols; c++) {
            state[r][c].enqueued = false;
            state[r][c].blackListed = false;
            state[r][c].costSoFar = LONG_MAX;   // Virtually ∞
            state[r][c].parent = makeLoc(start.row, start.col);
        }
    }
    state[start.row][start.col].enqueued = true;
    state[start.row][start.col].costSoFar = 0;
}

bool isLocEnqueued(Grid<LocStateT>& state, const Loc& l) {
    return state[l.row][l.col].enqueued;
}

void setLocEnqueued(Grid<LocStateT>& state, const Loc& l, bool b) {
    state[l.row][l.col].enqueued = b;
}

void setLocBlackListed(Grid<LocStateT>& state, const Loc& l, bool b) {
    state[l.row][l.col].blackListed = b;
}

void setLocCostSoFar(Grid<LocStateT>& state, const Loc& l, double cost) {
    state[l.row][l.col].costSoFar = cost;
}

void setLocParent(Grid<LocStateT>& state, const Loc& l, const Loc& parent) {
    state[l.row][l.col].parent = parent;
}

Set<Edge> createMaze(int numRows, int numCols) {
    // TODO: Fill this in!
    error("createMaze is not implemented yet.");
    return Set<Edge>();
}