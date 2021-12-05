#pragma once

#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>

enum class Category { Pop, Science, Sports, Rock };
constexpr std::string_view ToStringView(Category category)
{
  switch (category) {
    case Category::Pop: return "Pop";
    case Category::Science: return "Science";
    case Category::Sports: return "Sports";
    case Category::Rock: return "Rock";
  }
  throw std::runtime_error("Unknown category.");
}

struct Question {
  std::string text;
  Category category;
};
struct Answer {
};

/// Interface for custom turn mechanics.
class GameTurn {
 public:
  virtual ~GameTurn() = default;

  virtual int rollDice() const = 0;
  /// Move the player on the board.
  /// @param roll The value rolled by the player.
  /// @returns The new location of the player or nullopt if the player cannot move.
  virtual std::optional<int> movePlayer(int roll) = 0;
  /// Get the question corresponding to `location`.
  /// @returns The question or nullopt if there are no such questions.
  virtual std::optional<Question> readQuestion(int location) = 0;
  virtual Answer askQuestion(Question)                       = 0;
  virtual bool isAnswerCorrect(Answer)                       = 0;
  virtual void onCorrectAnswer()                             = 0;
  virtual void onIncorrectAnswer()                           = 0;
};

/// Base class for game implementations.
/// Implements the main game loop and allows for customization of the turn mechanics.
class Game {
 public:
  Game(int nPlayers);
  virtual ~Game() = default;

  void run();

 private:
  int getNextPlayerId(int currentPlayerId) const;

  /// This is called at the beginning of each turn.
  /// The id of the current player will be assigned to `playerId`.
  virtual std::unique_ptr<GameTurn> newTurn(int playerId) = 0;

  /// @returns true if the player with `playerId` has won the game.
  /// @note The main game loops exits if this returns true for the current player.
  virtual bool didPlayerWin(int playerId) const = 0;

  int nPlayers_;
};
