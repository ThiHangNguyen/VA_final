/**
 * @file filter.cpp
 * @brief Implémentation de filtres passe-bas pour le lissage des signaux AR.
 *
 * Ce fichier implémente des filtres passe-bas de type
 * moyenne exponentielle (EMA – Exponential Moving Average)
 * pour différentes dimensions :
 *
 * - LowPassFilter   : valeurs scalaires (float)
 * - LowPassFilter2D : vecteurs 2D (glm::vec2)
 * - LowPassFilter3D : vecteurs 3D (glm::vec3)
 *
 * Ces filtres sont utilisés dans le pipeline de réalité augmentée
 * afin de :
 * - réduire le bruit des mesures (détection, tracking)
 * - limiter les tremblements visuels
 * - améliorer la stabilité temporelle des poses et des mouvements
 *
 * Le paramètre alpha contrôle le compromis entre réactivité
 * (alpha élevé) et stabilité (alpha faible).
 *
 * @author Thi Hang NGUYEN
 */

#include "ar/filter.hpp"

namespace ar {

// ========================================
// LowPassFilter (scalaire)
// ========================================

LowPassFilter::LowPassFilter(float alpha)
    : m_alpha(alpha)
    , m_filteredValue(0.0f)
    , m_initialized(false)
{
}

float LowPassFilter::update(float newValue) {
    if (!m_initialized) {
        m_filteredValue = newValue;
        m_initialized = true;
        return m_filteredValue;
    }

    // EMA: filtered = alpha * new + (1 - alpha) * old
    m_filteredValue = m_alpha * newValue + (1.0f - m_alpha) * m_filteredValue;
    return m_filteredValue;
}

void LowPassFilter::reset(float value) {
    m_filteredValue = value;
    m_initialized = false;
}

// ========================================
// LowPassFilter2D
// ========================================

LowPassFilter2D::LowPassFilter2D(float alpha)
    : m_alpha(alpha)
    , m_filteredValue(0.0f, 0.0f)
    , m_initialized(false)
{
}

glm::vec2 LowPassFilter2D::update(const glm::vec2& newValue) {
    if (!m_initialized) {
        m_filteredValue = newValue;
        m_initialized = true;
        return m_filteredValue;
    }

    m_filteredValue = m_alpha * newValue + (1.0f - m_alpha) * m_filteredValue;
    return m_filteredValue;
}

void LowPassFilter2D::reset(const glm::vec2& value) {
    m_filteredValue = value;
    m_initialized = false;
}

// ========================================
// LowPassFilter3D
// ========================================

LowPassFilter3D::LowPassFilter3D(float alpha)
    : m_alpha(alpha)
    , m_filteredValue(0.0f, 0.0f, 0.0f)
    , m_initialized(false)
{
}

glm::vec3 LowPassFilter3D::update(const glm::vec3& newValue) {
    if (!m_initialized) {
        m_filteredValue = newValue;
        m_initialized = true;
        return m_filteredValue;
    }

    m_filteredValue = m_alpha * newValue + (1.0f - m_alpha) * m_filteredValue;
    return m_filteredValue;
}

void LowPassFilter3D::reset(const glm::vec3& value) {
    m_filteredValue = value;
    m_initialized = false;
}

} // namespace ar
