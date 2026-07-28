#ifndef MODELS_H
#define MODELS_H

#include <qivot.h>

// A neural network's "knowledge" is just numbers: a grid of weights and a short
// list of biases. We store them as plain rows, so you can literally open the
// database and read the brain.

/// One number in the weight GRID (a 2-D tensor / matrix): the strength of the
/// connection from input `inIdx` to output neuron `outIdx`.
class Weight : public QiModel {
    QI_MODEL
public:
    QiField<int>    outIdx;
    QiField<int>    inIdx;
    QiField<double> value;
};
QI_DECLARE_MODEL(Weight, "weight",
    QI_FIELD(outIdx), QI_FIELD(inIdx), QI_FIELD(value));

/// One number in the bias LIST (a 1-D tensor / vector): a nudge added to output
/// neuron `idx` after the weighted sum.
class Bias : public QiModel {
    QI_MODEL
public:
    QiField<int>    idx;
    QiField<double> value;
};
QI_DECLARE_MODEL(Bias, "bias",
    QI_FIELD(idx), QI_FIELD(value));

#endif // MODELS_H
