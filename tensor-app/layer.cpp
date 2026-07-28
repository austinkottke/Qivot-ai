#include "layer.h"
#include "models.h"

#include <qivot.h>
#include <QVariantMap>
#include <cmath>

static double squash(double z) { return 1.0 / (1.0 + std::exp(-z)); }

static const char *kOutName[2] = { "brightness", "warmth" };

Layer::Layer(QObject *parent) : QObject(parent) {}

void Layer::load() {
    // Read the weight grid and biases back out of SQLite.
    for (int o = 0; o < 2; o++) {
        for (int i = 0; i < 3; i++) {
            QiList<Weight> wl = QiQuery<Weight>()
                                  .filter(QiWhere("outIdx = ", o) && QiWhere("inIdx = ", i)).all();
            m_W[o][i] = wl.size() > 0 ? double(wl.at(0)->value) : 0.0;
        }
        QiList<Bias> bl = QiQuery<Bias>().filter(QiWhere("idx = ", o)).all();
        m_bias[o] = bl.size() > 0 ? double(bl.at(0)->value) : 0.0;
    }
    recompute();
}

void Layer::setRed(int v)   { v = qBound(0, v, 255); if (v == m_r) return; m_r = v; recompute(); emit changed(); }
void Layer::setGreen(int v) { v = qBound(0, v, 255); if (v == m_g) return; m_g = v; recompute(); emit changed(); }
void Layer::setBlue(int v)  { v = qBound(0, v, 255); if (v == m_b) return; m_b = v; recompute(); emit changed(); }

void Layer::recompute() {
    const double x[3] = { m_r / 255.0, m_g / 255.0, m_b / 255.0 };
    const char *inName[3] = { "red", "green", "blue" };
    const int   raw[3]    = { m_r, m_g, m_b };

    m_inputs.clear();
    for (int i = 0; i < 3; i++) {
        QVariantMap m;
        m["name"] = inName[i];
        m["norm"] = x[i];
        m["raw"]  = raw[i];
        m_inputs << m;
    }

    m_outputs.clear();
    for (int o = 0; o < 2; o++) {
        double sum = m_bias[o];
        QString expr;
        QVariantList w;
        for (int i = 0; i < 3; i++) {
            sum += m_W[o][i] * x[i];
            w << m_W[o][i];
            expr += QString::asprintf("%.2f×%.2f", x[i], m_W[o][i]);   // 0.12×0.30
            if (i < 2) expr += " + ";
        }
        QVariantMap m;
        m["name"]  = kOutName[o];
        m["w"]     = w;
        m["expr"]  = expr;
        m["sum"]   = sum;
        m["value"] = squash(sum);
        m_outputs << m;
    }
}
