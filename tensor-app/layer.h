#ifndef LAYER_H
#define LAYER_H

#include <QObject>
#include <QVariantList>

/// The backend QML talks to: one neural-network layer. It loads its weight grid
/// from SQLite, then for any input color computes the output numbers — exposing
/// every intermediate value so the UI can show the whole calculation live.
class Layer : public QObject {
    Q_OBJECT
    Q_PROPERTY(int red   READ red   WRITE setRed   NOTIFY changed)
    Q_PROPERTY(int green READ green WRITE setGreen NOTIFY changed)
    Q_PROPERTY(int blue  READ blue  WRITE setBlue  NOTIFY changed)
    Q_PROPERTY(QVariantList inputs  READ inputs  NOTIFY changed)  ///< the 3 colour numbers
    Q_PROPERTY(QVariantList outputs READ outputs NOTIFY changed)  ///< the layer's result

public:
    explicit Layer(QObject *parent = nullptr);

    void load();                       ///< read the weight grid + biases from the DB

    int red() const   { return m_r; }
    int green() const { return m_g; }
    int blue() const  { return m_b; }
    QVariantList inputs() const  { return m_inputs; }
    QVariantList outputs() const { return m_outputs; }

    void setRed(int v);
    void setGreen(int v);
    void setBlue(int v);

signals:
    void changed();

private:
    void recompute();

    int    m_r = 240, m_g = 150, m_b = 60;
    double m_W[2][3] = {{0,0,0},{0,0,0}};
    double m_bias[2] = {0, 0};
    QVariantList m_inputs;
    QVariantList m_outputs;
};

#endif // LAYER_H
