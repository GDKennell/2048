typedef unsigned long uint64_t;
typedef uint64_t board_t;

constant uint64_t UNUSED_BOARD = 0;
constant uint64_t UNUSED_HEUR = 0xFFFFFFFFFFFFFFF;

kernel void evaluate_outcomes(global board_t* input_boards,
                              global board_t* output_boards,
                              const size_t count)
{
  size_t index = get_global_id(0);
  size_t input_index = 4 * index;

  uint64_t bestMoveHeur = UNUSED_HEUR;
  for (uint64_t i = input_index; i < input_index + 4; ++i)
  {
    uint64_t moveHeur = input_boards[i];
    if (moveHeur != UNUSED_HEUR)
    {
      if (bestMoveHeur == UNUSED_HEUR || moveHeur > bestMoveHeur)
      {
        bestMoveHeur = moveHeur;
      }
    }
  }
  output_boards[index] = bestMoveHeur;
}
