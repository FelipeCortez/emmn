#ifndef NEXTPASSESVIEW_H
#define NEXTPASSESVIEW_H

#include <QWidget>
#include "tracker.h"

/*! Widget com visualização de próximas passagens
 *
 * @todo Adicionar interação com mouse
 */
class NextPassesView : public QWidget
{
    Q_OBJECT

public:
    /*! \brief Inicializa classe com ponteiro de satélites nullptr e habilitando detecção do mouse
     */
    explicit NextPassesView(QWidget *parent = 0);

    /*! \brief Desenha visualização de próximas passagens
     */
    void paintEvent(QPaintEvent *);

    /*! \brief Atribui ponteiro de lista de satélites
     */
    void setTrackers(QList<Tracker>* trackers);
private:
    QList<Tracker>* trackers; //!< Ponteiro com lista de satélites

signals:

public slots:
};

#endif // NEXTPASSESVIEW_H
