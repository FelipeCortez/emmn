#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include "ui_settings_dialog.h"

/*! Caixa de diálogo de configurações */
class SettingsDialog : public QDialog, public Ui::SettingsDialog {
    Q_OBJECT

public:
    /*! \brief Inicializa janela e conecta signals and slots
     */
    SettingsDialog(QWidget * parent = 0);

    /*! \brief Atualiza componentes de interface com configurações armazenadas no sistema
     */
    void updateWithSettings();
public slots:
    /*! \brief Slot chamado ao clicar em "Ok". Salva configurações no sistema
     */
     void accept();
};

#endif // SETTINGSDIALOG_H
