#ifndef AR_FILTER_HPP
#define AR_FILTER_HPP

#include <glm/glm.hpp>

namespace ar {

/**
 * @brief Filtre passe-bas (EMA - Exponential Moving Average) pour 1 valeur scalaire
 *
 * Ce filtre permet de réduire le jitter (tremblotement) en lissant les variations
 * rapides tout en conservant les tendances à long terme.
 *
 * Plus alpha est petit, plus le filtrage est fort (moins de jitter, plus de latence).
 * Plus alpha est grand, plus le signal est réactif (plus de jitter, moins de latence).
 */
class LowPassFilter {
public:
    /**
     * @param alpha Coefficient de lissage [0,1]. Défaut: 0.2 (bon compromis jitter/latence)
     */
    explicit LowPassFilter(float alpha = 0.2f);

    /**
     * @brief Applique le filtre sur une nouvelle valeur
     * @param newValue Nouvelle valeur brute à filtrer
     * @return Valeur filtrée
     */
    float update(float newValue);

    /**
     * @brief Réinitialise le filtre avec une valeur de départ
     * @param value Valeur initiale
     */
    void reset(float value = 0.0f);

    /**
     * @brief Obtient la dernière valeur filtrée
     */
    float getValue() const { return m_filteredValue; }

private:
    float m_alpha;           ///< Coefficient de lissage
    float m_filteredValue;   ///< Valeur filtrée actuelle
    bool m_initialized;      ///< Premier appel ?
};

/**
 * @brief Filtre passe-bas pour un vecteur 2D (ex: accélération X,Y)
 */
class LowPassFilter2D {
public:
    explicit LowPassFilter2D(float alpha = 0.2f);

    /**
     * @brief Applique le filtre sur un nouveau vecteur 2D
     * @param newValue Nouvelle valeur brute (x, y)
     * @return Valeur filtrée (x, y)
     */
    glm::vec2 update(const glm::vec2& newValue);

    /**
     * @brief Réinitialise le filtre
     */
    void reset(const glm::vec2& value = glm::vec2(0.0f));

    glm::vec2 getValue() const { return m_filteredValue; }

private:
    float m_alpha;
    glm::vec2 m_filteredValue;
    bool m_initialized;
};

/**
 * @brief Filtre passe-bas pour un vecteur 3D (ex: position, vitesse)
 */
class LowPassFilter3D {
public:
    explicit LowPassFilter3D(float alpha = 0.2f);

    glm::vec3 update(const glm::vec3& newValue);
    void reset(const glm::vec3& value = glm::vec3(0.0f));
    glm::vec3 getValue() const { return m_filteredValue; }

private:
    float m_alpha;
    glm::vec3 m_filteredValue;
    bool m_initialized;
};

} // namespace ar

#endif // AR_FILTER_HPP
