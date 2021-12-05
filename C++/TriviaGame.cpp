#include "TriviaGame.h"

#include <array>

namespace {
Category categoryForField(int field)
{
  static constexpr const std::array<Category, 4> categories = {
      Category::Pop, Category::Science, Category::Sports, Category::Rock};
  return categories[field % categories.size()];
}
}  // namespace

TriviaGame::TriviaGame(std::vector<Player> players, QuestionPool questionPool, std::ostream& logger)
    : Game(players.size())
    , players_(std::move(players))
    , questionPool_(std::move(questionPool))
    , logger_(logger)
{
}

std::optional<TriviaGame> TriviaGame::Create(std::vector<std::string> playerNames,
                                             QuestionPool questionPool,
                                             std::ostream& logger)
{
  if (playerNames.size() < 2)
    return std::nullopt;

  std::vector<Player> players;
  players.reserve(playerNames.size());

  for (auto&& name : playerNames) {
    players.emplace_back(std::move(name), Player::State{0, 0, false});
    logger << players.back().name << " was added\n";
    logger << "They are player number " << players.size() << "\n";
  }

  return TriviaGame(std::move(players), std::move(questionPool), logger);
}

std::unique_ptr<GameTurn> TriviaGame::newTurn(int playerId)
{
  return std::make_unique<TriviaGameTurn>(players_[playerId], questionPool_, logger_);
}

bool TriviaGame::didPlayerWin(int playerId) const
{
  return players_[playerId].state.coins == 6;
}

TriviaGameTurn::TriviaGameTurn(Player& player, QuestionPool& questionPool, std::ostream& logger)
    : player_(player), questionPool_(questionPool), logger_(logger)
{
}

int TriviaGameTurn::rollDice() const
{
  const int roll = rand() % 5 + 1;
  logger_ << player_.name << " is the current player\n";
  logger_ << "They have rolled a " << roll << "\n";
  return roll;
}

std::optional<int> TriviaGameTurn::movePlayer(int roll)
{
  if (player_.state.inPenaltyBox) {
    if (roll % 2 != 0) {
      player_.state.inPenaltyBox = false;
      logger_ << player_.name << " is getting out of the penalty box\n";
    } else {
      logger_ << player_.name << " is not getting out of the penalty box\n";
      return std::nullopt;
    }
  }

  player_.state.field = (player_.state.field + roll) % 12;
  logger_ << player_.name << "'s new location is " << player_.state.field << "\n";
  return player_.state.field;
}

Question TriviaGameTurn::readQuestion(int location)
{
  const auto category = categoryForField(location);
  const auto question = nextQuestion(category);

  return {question, category};
}

Answer TriviaGameTurn::askQuestion(Question question)
{
  logger_ << "The category is " << ToStringView(question.category) << "\n";
  logger_ << question.text << "\n";
  return {};
}

bool TriviaGameTurn::isAnswerCorrect(Answer /*answer*/)
{
  return rand() % 9 != 7;
}

void TriviaGameTurn::onCorrectAnswer()
{
  player_.state.coins++;
  logger_ << "Answer was correct!!!!\n";
  logger_ << player_.name << " now has " << player_.state.coins << " Gold Coins.\n";
}

void TriviaGameTurn::onIncorrectAnswer()
{
  logger_ << "Question was incorrectly answered\n";
  logger_ << player_.name + " was sent to the penalty box\n";
  player_.state.inPenaltyBox = true;
}

std::string TriviaGameTurn::nextQuestion(Category category)
{
  auto& questionGroup = questionPool_[category];
  auto question       = questionGroup.front();
  questionGroup.pop_front();
  return question;
}
