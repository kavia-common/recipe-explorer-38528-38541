#ifndef MAIN_APP_H
#define MAIN_APP_H

#include <QMainWindow>
#include <QTabWidget>
#include <QListWidget>
#include <QWidget>
#include <QString>
#include <QVector>
#include <QMap>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QScrollArea>
#include <QVBoxLayout>

/*
    Ocean Professional Theme
    primary:   #2563EB
    secondary: #F59E0B
    success:   #F59E0B
    error:     #EF4444
    background:#f9fafb
    surface:   #ffffff
    text:      #111827
*/

// PUBLIC_INTERFACE
struct Recipe {
    /** Recipe data model representing a single recipe item. */
    QString id;
    QString title;
    QString imageUrl;
    QStringList ingredients;
    QStringList steps;
    QStringList tags;
    bool isFavorite{false};
};

// PUBLIC_INTERFACE
class MockRecipeProvider {
    /** In-memory mock data provider for recipes. */
public:
    MockRecipeProvider();
    const QVector<Recipe>& all() const;
    QVector<Recipe> search(const QString& term) const;
    void toggleFavorite(const QString& id);
    QVector<Recipe> favorites() const;
    const Recipe* findById(const QString& id) const;

private:
    QVector<Recipe> m_recipes;
    QMap<QString, int> m_indexById;
};

class BrowseScreen;
class SearchScreen;
class FavoritesScreen;
class RecipeDetailsScreen;

// PUBLIC_INTERFACE
class MainWindow : public QMainWindow {
    /** Main application window containing tab navigation and theme setup. */
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

    // Navigation to details screen with data
    void openRecipeDetails(const Recipe& recipe);

    MockRecipeProvider& dataProvider();

private:
    void setupUi();
    void applyGlobalStyle();

    QTabWidget* m_tabs{nullptr};
    BrowseScreen* m_browse{nullptr};
    SearchScreen* m_search{nullptr};
    FavoritesScreen* m_favorites{nullptr};
    MockRecipeProvider m_provider;
};

// PUBLIC_INTERFACE
class BrowseScreen : public QWidget {
    /** Browse screen showing list of mock recipes. */
    Q_OBJECT
public:
    explicit BrowseScreen(MainWindow* host, QWidget* parent = nullptr);
    void refresh();

private:
    MainWindow* m_host{nullptr};
    QListWidget* m_list{nullptr};
};

// PUBLIC_INTERFACE
class SearchScreen : public QWidget {
    /** Search screen with input and placeholder results. */
    Q_OBJECT
public:
    explicit SearchScreen(MainWindow* host, QWidget* parent = nullptr);

private slots:
    void onSearch();

private:
    MainWindow* m_host{nullptr};
    QLineEdit* m_input{nullptr};
    QListWidget* m_results{nullptr};
};

// PUBLIC_INTERFACE
class FavoritesScreen : public QWidget {
    /** Favorites screen with empty state and list. */
    Q_OBJECT
public:
    explicit FavoritesScreen(MainWindow* host, QWidget* parent = nullptr);
    void refresh();

private:
    MainWindow* m_host{nullptr};
    QLabel* m_empty{nullptr};
    QListWidget* m_list{nullptr};
};

// PUBLIC_INTERFACE
class RecipeDetailsScreen : public QWidget {
    /** Recipe Details screen to show selected recipe fields. */
    Q_OBJECT
public:
    explicit RecipeDetailsScreen(MainWindow* host, QWidget* parent = nullptr);
    void setRecipe(const Recipe& recipe);

private slots:
    void onToggleFavorite();

private:
    MainWindow* m_host{nullptr};
    Recipe m_recipe;
    QLabel* m_title{nullptr};
    QLabel* m_tags{nullptr};
    QTextEdit* m_ingredients{nullptr};
    QTextEdit* m_steps{nullptr};
    QPushButton* m_favoriteBtn{nullptr};
};

#endif // MAIN_APP_H