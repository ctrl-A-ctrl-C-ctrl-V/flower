# flower.c User Manual

## Overview
FLOWER (`FL`oorplanner `W`ith `E`fficient `R`outing) is a floorplanning and routing application created in 1992 as part of my MTech thesis at Indian Institute of Technology in 1992. `flower.c` is the main body of the work. It is a standalone C program for simple chip block placement and first-layer routing on a fixed 2D grid. It reads block sizes and connection data from the user, computes a floorplan, places blocks, generates routing space, and attempts to connect nets on a single metallization layer.

---

## Key Features
- Interactive command-line input
- Supports up to:
  - 25 blocks
  - 50 distinct connections
- Uses an `80 x 80` grid for layout and routing
- Generates a text-based layout view
- Detects insufficient area and aborts if routing space is too small

---

## Build Instructions
Example build commands:

- Visual Studio / cl:
  - `cl /Zi /W3 flower.c`
- Clang on Windows with MSVC compatibility:
  - `clang -std=c17 flower.c -o flower.exe`

---

## Run Instructions
Run the compiled executable from a terminal:

- Windows:
  - `flower.exe`

The program is interactive and will prompt for block and connection data.

---

## Input Format
The program asks for:

1. Number of blocks
2. For each block:
   - length
   - width
   - number of connections
   - each connection number

Connection numbers are assumed to start at `1`. The program tracks the highest connection number entered and uses that as the total number of nets.

### Example Input Sequence
```
How many blocks do you have? 3
The length of the 1th block : 4
The width of the 1th block : 3
How many connections are there with this block ? 2
Input the connections :
    1th connection : 1
    2th connection : 2
...
```

---

## Output
After computing placement and routing, the program prints a grid and overall die area.

### Display symbols
- `.` : empty routable cell
- `#` : blocked cell or block boundary
- `A`, `B`, `C`, ... : connection endpoints and nets on the first metal layer
- `^` : routed wire segments

### Summary output
- Total die area printed as:
  - `Total die area is X X sq. unit`
- Connection completion status:
  - If a net is not fully connected, it reports:
    - `N out of M connection-segment achieved for conn. K`

---

## How It Works
The program implements the following major stages:

1. **Read Input**
   - Load block dimensions and connections.
2. **Build connection map**
   - For each connection, collect the blocks attached to it.
3. **Compute block area and density**
   - Reject the design if total block area is too large relative to the grid.
4. **Placement**
   - Recursively partition the blocks based on connection affinity.
   - Use budgeted subregions to assign block center coordinates.
5. **Grid formation**
   - Map placed blocks to the `80 x 80` routing grid.
   - Place net terminals at block edges.
6. **Routing scan**
   - Perform raster scan propagation from terminals.
   - Connect nets when wavefronts meet.
   - Use backtracking to mark completed routes.
7. **Cleanup**
   - Remove redundant routes and straighten segments where possible.
   - The `compress()` optimization is present but currently disabled.
8. **Display**
   - Print the routed layout visually.

---

## Assumptions and Limits
- Fixed grid size: `XDIM = 80`, `YDIM = 80`
- Maximum blocks: `MAX_BLOCK = 25`
- Maximum connections: `MAX_CON = 50`
- The program only attempts a single metal layer routing strategy
- No command-line options: only keyboard input
- No advanced input validation for malformed or duplicate connection entries
- If total block area is more than half the grid area, it aborts

---

## Notes
- The program was the MS thesis project more than 30 years back by the author. It is a demonstration of basic floorplanning and routing, not a production EDA tool.
- It is best suited for small block/cell sets with relatively simple net structure.
- Because the grid and algorithm are fixed, larger or highly connected designs may not route successfully.

---

## Practical Tips
- Keep block counts low and connection densities moderate.
- Use small block dimensions so the layout fits comfortably in the `80 x 80` grid.
- If you see many `^` symbols, the routing is active; if nets remain incomplete, the program reports it.
