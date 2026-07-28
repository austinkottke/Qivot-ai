#ifndef MODELS_H
#define MODELS_H

#include <qivot.h>

/// One number in the weight GRID (a 2-D tensor): the connection strength from
/// input `inIdx` to output neuron `outIdx`.
class Weight : public QiModel {
    QI_MODEL
public:
    QiField<int>    outIdx;
    QiField<int>    inIdx;
    QiField<double> value;
};
QI_DECLARE_MODEL(Weight, "weight",
    QI_FIELD(outIdx), QI_FIELD(inIdx), QI_FIELD(value));

/// One number in the bias LIST (a 1-D tensor): a nudge for output neuron `idx`.
class Bias : public QiModel {
    QI_MODEL
public:
    QiField<int>    idx;
    QiField<double> value;
};
QI_DECLARE_MODEL(Bias, "bias",
    QI_FIELD(idx), QI_FIELD(value));

#endif // MODELS_H
