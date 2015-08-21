# 2048 by Grant Kennell
# README file written August 2015
2048 solver

This is a simulation of the game 2048 and an algorithm to play the game.

Branches:
    There are many branches on this repository from different algorithms and optimizations which I have tried to
    implement previously. The most successful branch and the one that is currently being maintained is called 
    expectimax_refactor because it uses the expectimax algorithm and I am in the process of refactoring it for
    readability.
    
Credit:
    Throughout the development of this project I consulted descriptions of other solutions that have been written for
    this game. Many of the optimizations I employed which were crucial to the success of the project were inspired by
    these other solutions. At the time of writing a year has passed since I did that research so the exact sources
    elude my memory but I will make an effort to find them and cite them.
    
Algorithm:
    The basic algorithm for the game playing AI is expecti-max. Expecti-max essentially means evaluating all possible
    outcomes of each possible move and chosing the move whose average outcome is the best. This algorithm fits the 
    game problem well because the outcomes of a move (the placement of a new tile) occur at random, so it makes
    sense to average those outcomes.
    
    The algorithm comes up with a numerical evaluation of each move by examining it recursively. For each move,
    it computes the resulting board. For each resulting board it examines all possible placements of new tiles
    (2 or 4, at any open space). From each possible placement, it examines all possible moves. Etc.
    This forms a tree of possible outcomes. The tree has a branching factor between 1 and 4 inclusive on examining
    possible moves, and between 2 and 28 on examining possible new tile placements. As you can see, the number of
    moves to examine grows exponentially with the depth of the tree, so the depth must be limited to a relatively
    small number. However if the tree is not examined to sufficient depth the algorithm will not make a good
    move decision.
    
Optimizations:
    As outlined in the Algorithm section, the deeper in the tree the algorithm can look the better a decision it can
    make. Initial implementations of the algorithm ran very slowly and had very unimpressive results. It was through
    optimizations of the code and thus improvements in computation speed that the algorithm's results improved.
    At the time of writing, the algorithm is able to complete a run in several minutes (1-2 for a short run, 5-10 on
    a particularly good one), it is able to beat the game (reach the 2048 tile) about 75% of the time, and has
    attained a high-score of 70108 (with both a 4096 and 2048 tile).
    
    Here is an explanation of some of the most critical optimizations:
    
    64-bit Board Representation:
        The board is the most used data structure in the code and is duplicated, manipulated, and passed around
        very frequently. Minimizing the size of the board data structure yields significant improvements in
        efficiency.
       
        Because each tile is a power of 2, each may be stored as the exponent of 2 rather than the value itself -
        2 may be stored as 1, 4 as 2, 8 as 3... 2048 as 11 etc. To encode up to 2048 (2^11), we need just
        ceil(log2(11)) == 4 bits. With 4 bits per tile and a 4x4 = 16 tile board, we only need 16x4 = 64 bits to
        store the board.
       
        The SmallBoard class represents the board with a single uint64_t, a 64-bit integer. It encapsulates all
        the required bit-shifting logic to access the board with x,y coordinates.
        
    Pre-computation of row-shifting
        The operation of executing a move is a very common operation and requires non-trivial computation.
        Shifting all tiles to one edge of the board, combining adjacent tiles, and tallying score for combined
        tiles becomes a significant portion of the computation performed by the algorithm as it examines thousands
        of possible moves.
        
        This computation can be entirely eliminated by pre-computation. Before running the algorithm we iterate over
        all possible rows of 4 tiles, perform the shifting and combining of tiles, and record the result. We store
        a all of these results in order so that we can take the integer representation of a row, index into the
        precomputation array, and get back the integer representation of the shifted row. 
        
        Computing the result of shifting all possible rows might sound like a lot at first but it's actually quite
        manageable. One tile is encoded in 4 bits and a row of 4 tiles is encoded in 16 bits, so the number of
        possible rows is 2^16 = 65,536. If we store one 4-byte integer of data related to each possible row in a 
        file, we end up with a file of only roughly 260kB. The move_precompute.cpp file holds the function which
        writes two such files, one for each direction (left.bin and right.bin). The main function calls this code
        to create these files if they don't already exist and then loads the data from the files into an array for
        use in the algorithm.
        
    Pre-computation of empty tile counts
        Another key computation in the algorithm is counting the number of empty tiles on a board. The number of
        empty tiles on a board is used as the heuristic evaluation, trying to get to a board with the most empty
        space, so this is checked very often. While iterating over the tiles of a row and counting how many are empty
        is a relatively simple task, optimizing it down to a table lookup like is done with the shifting pre-
        computation is a significant performance boost.
        
        Similar to the pre-computation of move evaluation, pre-computation is done by iterating over all possible
        rows, counting the number of empty tiles, and storing that value in an array for lookup by the integer
        representation of the row. The same method is also employed of storing this data to a file (numempty.bin)
        and only recreating it if it does not exist yet.
    
    
    More will be added to this ReadMe soon
