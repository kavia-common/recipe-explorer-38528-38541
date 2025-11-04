#include "mainApp.h"

#include <QApplication>
#include <QStyleHints>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSpacerItem>
#include <QRegularExpression>

// ======================== MockRecipeProvider ========================
MockRecipeProvider::MockRecipeProvider() {
    // Seed with a small set of recipes
    QVector<Recipe> seed = {
        { "r1", "Ocean Grilled Salmon", "https://picsum.photos/seed/salmon/600/400",
          {"2 salmon fillets", "1 tbsp olive oil", "Salt", "Pepper", "Lemon wedges"},
          {"Preheat grill to medium-high", "Brush salmon with oil", "Season with salt and pepper", "Grill 4-5 min per side", "Serve with lemon"},
          {"seafood", "healthy", "grill"}, false },
        { "r2", "Amber Roasted Veggies", "https://picsum.photos/seed/veggies/600/400",
          {"2 cups mixed vegetables", "2 tbsp olive oil", "1 tsp paprika", "Salt"},
          {"Preheat oven to 200C", "Toss veggies with oil and paprika", "Roast 20-25 minutes", "Season to taste"},
          {"vegetarian", "sides", "quick"}, true },
        { "r3", "Blueberry Oatmeal", "https://picsum.photos/seed/oat/600/400",
          {"1 cup oats", "2 cups water or milk", "1/2 cup blueberries", "Honey"},
          {"Bring liquid to boil", "Stir in oats and simmer 5 minutes", "Top with blueberries and honey"},
          {"breakfast", "healthy", "gluten-optional"}, false }
    };
    m_recipes = seed;
    for (int i = 0; i < m_recipes.size(); ++i) {
        m_indexById[m_recipes[i].id] = i;
    }
}

const QVector<Recipe>& MockRecipeProvider::all() const { return m_recipes; }

QVector<Recipe> MockRecipeProvider::search(const QString& term) const {
    if (term.trimmed().isEmpty()) return {};
    QVector<Recipe> out;
    const auto rx = QRegularExpression(QRegularExpression::escape(term.trimmed()), QRegularExpression::CaseInsensitiveOption);
    for (const auto& r : m_recipes) {
        if (r.title.contains(rx) || r.tags.join(" ").contains(rx)) {
            out.push_back(r);
        }
    }
    return out;
}

void MockRecipeProvider::toggleFavorite(const QString& id) {
    if (!m_indexById.contains(id)) return;
    auto idx = m_indexById[id];
    m_recipes[idx].isFavorite = !m_recipes[idx].isFavorite;
}

QVector<Recipe> MockRecipeProvider::favorites() const {
    QVector<Recipe> out;
    for (const auto& r : m_recipes) if (r.isFavorite) out.push_back(r);
    return out;
}

const Recipe* MockRecipeProvider::findById(const QString& id) const {
    if (!m_indexById.contains(id)) return nullptr;
    return &m_recipes[m_indexById[id]];
}

// ======================== RecipeDetailsScreen ========================
RecipeDetailsScreen::RecipeDetailsScreen(MainWindow* host, QWidget* parent)
    : QWidget(parent), m_host(host)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 16, 16, 16);
    root->setSpacing(12);

    m_title = new QLabel(this);
    m_title->setStyleSheet("QLabel { font-size: 20px; font-weight: 700; color: #111827; }");

    m_tags = new QLabel(this);
    m_tags->setStyleSheet("QLabel { color: #6B7280; }");

    m_favoriteBtn = new QPushButton("Add to Favorites", this);
    m_favoriteBtn->setCursor(Qt::PointingHandCursor);
    m_favoriteBtn->setStyleSheet(
        "QPushButton { background-color: #2563EB; color: white; padding: 8px 12px; border-radius: 8px; }"
        "QPushButton:hover { background-color: #1E40AF; }"
    );
    connect(m_favoriteBtn, &QPushButton::clicked, this, &RecipeDetailsScreen::onToggleFavorite);

    auto* ingredientsLabel = new QLabel("Ingredients", this);
    ingredientsLabel->setStyleSheet("QLabel { font-weight: 600; color: #111827; }");

    m_ingredients = new QTextEdit(this);
    m_ingredients->setReadOnly(true);
    m_ingredients->setStyleSheet("QTextEdit { background: #ffffff; border: 1px solid #E5E7EB; border-radius: 8px; }");

    auto* stepsLabel = new QLabel("Steps", this);
    stepsLabel->setStyleSheet("QLabel { font-weight: 600; color: #111827; }");

    m_steps = new QTextEdit(this);
    m_steps->setReadOnly(true);
    m_steps->setStyleSheet("QTextEdit { background: #ffffff; border: 1px solid #E5E7EB; border-radius: 8px; }");

    root->addWidget(m_title);
    root->addWidget(m_tags);
    root->addWidget(m_favoriteBtn);
    root->addSpacing(8);
    root->addWidget(ingredientsLabel);
    root->addWidget(m_ingredients);
    root->addWidget(stepsLabel);
    root->addWidget(m_steps);
    setStyleSheet("background-color: #f9fafb;");
}

void RecipeDetailsScreen::setRecipe(const Recipe& recipe) {
    m_recipe = recipe;
    m_title->setText(recipe.title);
    m_tags->setText(QString("Tags: %1").arg(recipe.tags.join(", ")));
    m_ingredients->setText("- " + recipe.ingredients.join("\n- "));
    int i = 1;
    QString steps;
    for (const auto& s : recipe.steps) {
        steps += QString::number(i++) + ". " + s + "\n";
    }
    m_steps->setText(steps.trimmed());
    m_favoriteBtn->setText(recipe.isFavorite ? "Remove from Favorites" : "Add to Favorites");
}

void RecipeDetailsScreen::onToggleFavorite() {
    if (!m_host) return;
    m_host->dataProvider().toggleFavorite(m_recipe.id);
    const Recipe* updated = m_host->dataProvider().findById(m_recipe.id);
    if (updated) {
        setRecipe(*updated);
    }
}

// ======================== BrowseScreen ========================
BrowseScreen::BrowseScreen(MainWindow* host, QWidget* parent)
    : QWidget(parent), m_host(host)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    auto* header = new QLabel("Browse Recipes", this);
    header->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    header->setStyleSheet("QLabel { font-size: 18px; font-weight: 600; color: #111827; padding: 8px; }");

    m_list = new QListWidget(this);
    m_list->setStyleSheet(
        "QListWidget { background: #ffffff; border: 1px solid #E5E7EB; border-radius: 10px; }"
        "QListWidget::item { padding: 10px; }"
        "QListWidget::item:selected { background: #DBEAFE; }"
    );
    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem* item){
        if (!m_host) return;
        const QString id = item->data(Qt::UserRole).toString();
        const Recipe* r = m_host->dataProvider().findById(id);
        if (r) {
            m_host->openRecipeDetails(*r);
        }
    });

    root->addWidget(header);
    root->addWidget(m_list);
    setStyleSheet("background-color: #f9fafb;");
    refresh();
}

void BrowseScreen::refresh() {
    if (!m_host) return;
    m_list->clear();
    for (const auto& r : m_host->dataProvider().all()) {
        auto* item = new QListWidgetItem(QString("%1%2").arg(r.title, r.isFavorite ? "  ⭐" : ""));
        item->setData(Qt::UserRole, r.id);
        m_list->addItem(item);
    }
}

// ======================== SearchScreen ========================
SearchScreen::SearchScreen(MainWindow* host, QWidget* parent)
    : QWidget(parent), m_host(host)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    auto* header = new QLabel("Search", this);
    header->setStyleSheet("QLabel { font-size: 18px; font-weight: 600; color: #111827; padding: 8px; }");

    auto* row = new QHBoxLayout();
    m_input = new QLineEdit(this);
    m_input->setPlaceholderText("Search recipes or tags...");
    m_input->setStyleSheet("QLineEdit { background: #ffffff; border: 1px solid #E5E7EB; border-radius: 8px; padding: 8px; }");

    auto* btn = new QPushButton("Go", this);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setStyleSheet(
        "QPushButton { background-color: #2563EB; color: white; padding: 8px 12px; border-radius: 8px; }"
        "QPushButton:hover { background-color: #1E40AF; }"
    );
    connect(btn, &QPushButton::clicked, this, &SearchScreen::onSearch);

    row->addWidget(m_input);
    row->addWidget(btn);

    m_results = new QListWidget(this);
    m_results->setStyleSheet(
        "QListWidget { background: #ffffff; border: 1px solid #E5E7EB; border-radius: 10px; }"
        "QListWidget::item { padding: 10px; }"
        "QListWidget::item:selected { background: #DBEAFE; }"
    );
    connect(m_results, &QListWidget::itemClicked, this, [this](QListWidgetItem* item){
        if (!m_host) return;
        const QString id = item->data(Qt::UserRole).toString();
        const Recipe* r = m_host->dataProvider().findById(id);
        if (r) m_host->openRecipeDetails(*r);
    });

    root->addWidget(header);
    root->addLayout(row);
    root->addWidget(m_results);
    setStyleSheet("background-color: #f9fafb;");
}

void SearchScreen::onSearch() {
    if (!m_host) return;
    const QString term = m_input->text();
    m_results->clear();
    const auto list = m_host->dataProvider().search(term);
    if (list.isEmpty()) {
        m_results->addItem(new QListWidgetItem("No results"));
        return;
    }
    for (const auto& r : list) {
        auto* item = new QListWidgetItem(QString("%1%2").arg(r.title, r.isFavorite ? "  ⭐" : ""));
        item->setData(Qt::UserRole, r.id);
        m_results->addItem(item);
    }
}

// ======================== FavoritesScreen ========================
FavoritesScreen::FavoritesScreen(MainWindow* host, QWidget* parent)
    : QWidget(parent), m_host(host)
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);

    auto* header = new QLabel("Favorites", this);
    header->setStyleSheet("QLabel { font-size: 18px; font-weight: 600; color: #111827; padding: 8px; }");

    m_empty = new QLabel("No favorites yet.\nBrowse recipes and add some!", this);
    m_empty->setAlignment(Qt::AlignCenter);
    m_empty->setStyleSheet("QLabel { color: #6B7280; padding: 24px; }");

    m_list = new QListWidget(this);
    m_list->setStyleSheet(
        "QListWidget { background: #ffffff; border: 1px solid #E5E7EB; border-radius: 10px; }"
        "QListWidget::item { padding: 10px; }"
        "QListWidget::item:selected { background: #DBEAFE; }"
    );
    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem* item){
        if (!m_host) return;
        const QString id = item->data(Qt::UserRole).toString();
        const Recipe* r = m_host->dataProvider().findById(id);
        if (r) m_host->openRecipeDetails(*r);
    });

    root->addWidget(header);
    root->addWidget(m_empty);
    root->addWidget(m_list);
    setStyleSheet("background-color: #f9fafb;");
    refresh();
}

void FavoritesScreen::refresh() {
    if (!m_host) return;
    const auto favs = m_host->dataProvider().favorites();
    m_list->setVisible(!favs.isEmpty());
    m_empty->setVisible(favs.isEmpty());

    m_list->clear();
    for (const auto& r : favs) {
        auto* item = new QListWidgetItem(QString("%1  ⭐").arg(r.title));
        item->setData(Qt::UserRole, r.id);
        m_list->addItem(item);
    }
}

// ======================== MainWindow ========================
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    applyGlobalStyle();
    setupUi();
    setWindowTitle("Recipe Explorer");
    resize(900, 600);
}

void MainWindow::applyGlobalStyle() {
    // Subtle global stylesheet to apply theme and rounded corners, shadows on surface
    QString style =
        "QMainWindow { background-color: #f9fafb; }"
        "QTabWidget::pane { border: 1px solid #E5E7EB; border-radius: 10px; background: #ffffff; }"
        "QTabBar::tab { background: #ffffff; color: #111827; padding: 8px 14px; border: 1px solid #E5E7EB; border-bottom: none; border-top-left-radius: 8px; border-top-right-radius: 8px; }"
        "QTabBar::tab:selected { background: #DBEAFE; color: #111827; }"
        "QTabBar::tab:hover { background: #EFF6FF; }";
    setStyleSheet(style);
}

void MainWindow::setupUi() {
    m_tabs = new QTabWidget(this);
    setCentralWidget(m_tabs);

    m_browse = new BrowseScreen(this, this);
    m_search = new SearchScreen(this, this);
    m_favorites = new FavoritesScreen(this, this);

    m_tabs->addTab(m_browse, "Browse");
    m_tabs->addTab(m_search, "Search");
    m_tabs->addTab(m_favorites, "Favorites");

    // When switching to favorites, refresh to reflect latest changes
    connect(m_tabs, &QTabWidget::currentChanged, this, [this](int index){
        QWidget* w = m_tabs->widget(index);
        if (w == m_favorites) m_favorites->refresh();
        if (w == m_browse) m_browse->refresh();
    });
}

void MainWindow::openRecipeDetails(const Recipe& recipe) {
    // Create a new details screen and present in a tab for simplicity
    auto* details = new RecipeDetailsScreen(this, this);
    details->setRecipe(recipe);

    int existingIndex = -1;
    for (int i = 0; i < m_tabs->count(); ++i) {
        if (m_tabs->tabText(i) == "Details") { existingIndex = i; break; }
    }
    if (existingIndex >= 0) {
        m_tabs->removeTab(existingIndex);
    }
    int idx = m_tabs->addTab(details, "Details");
    m_tabs->setCurrentIndex(idx);

    // When closing the app, Qt will clean up. For a more advanced stack, we could manage a stack widget.
}

MockRecipeProvider& MainWindow::dataProvider() { return m_provider; }

// ======================== main() ========================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
