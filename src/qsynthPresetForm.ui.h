// qsynthPresetForm.ui.h
//
// ui.h extension file, included from the uic-generated form implementation.
/****************************************************************************
   Copyright (C) 2003, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*****************************************************************************/

#include <qfileinfo.h>

#include "config.h"


// Kind of constructor.
void qsynthPresetForm::init()
{
    m_pSynth = NULL;
    m_iChan  = 0;

    // No default sorting, initially.
    BankListView->setSorting(BankListView->columns() + 1);
    ProgListView->setSorting(ProgListView->columns() + 1);
}


// Kind of destructor.
void qsynthPresetForm::destroy()
{
}


// Dialog setup loader.
void qsynthPresetForm::setup ( fluid_synth_t *pSynth, int iChan )
{
    m_pSynth = pSynth;
    m_iChan  = iChan;

    // set the proper caption...
    setCaption(tr("Channel") + " " + QString::number(m_iChan + 1));

    // Load bank list from actual synth stack...
    BankListView->clear();
    QListViewItem *pBankItem = NULL;
    QListViewItem *pProgItem = NULL;
    fluid_preset_t preset;
    // For all soundfonts (in reversed stack order) fill the available banks...
    int cSoundFonts = ::fluid_synth_sfcount(m_pSynth);
    for (int i = 0; i < cSoundFonts; i++) {
        fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont(m_pSynth, i);
        if (pSoundFont) {
            pSoundFont->iteration_start(pSoundFont);
            while (pSoundFont->iteration_next(pSoundFont, &preset)) {
                int iBank = preset.get_banknum(&preset);
                if (!findBankItem(iBank)) {
                    pBankItem = new QListViewItem(BankListView, pBankItem);
                    if (pBankItem)
                        pBankItem->setText(0, QString::number(iBank));
                }
            }
        }
    }

    // Set the selected bank.
    pBankItem = NULL;
    fluid_preset_t *pPreset = ::fluid_synth_get_channel_preset(m_pSynth, m_iChan);
    if (pPreset)
        pBankItem = findBankItem(pPreset->get_banknum(pPreset));
    BankListView->setSelected(pBankItem, true);
    BankListView->ensureItemVisible(pBankItem);
    bankChanged();

    // Set the selected program.
    pProgItem = NULL;
    if (pPreset)
        pProgItem = findProgItem(pPreset->get_num(pPreset));
    ProgListView->setSelected(pProgItem, true);
    ProgListView->ensureItemVisible(pProgItem);
}


// Stabilize current state form.
void qsynthPresetForm::stabilizeForm()
{
    OkPushButton->setEnabled(validateForm());
}


// Validate form fields.
bool qsynthPresetForm::validateForm()
{
    bool bValid = true;

    bValid = bValid && (BankListView->selectedItem() != NULL);
    bValid = bValid && (ProgListView->selectedItem() != NULL);

    return bValid;
}

// Validate form fields and accept it valid.
void qsynthPresetForm::acceptForm()
{
    if (validateForm()) {
        // Unload from current selected dialog items.
        int iBank = (BankListView->selectedItem())->text(0).toInt();
        int iProg = (ProgListView->selectedItem())->text(0).toInt();
        // And set it right away...
        ::fluid_synth_bank_select(m_pSynth, m_iChan, iBank);
        ::fluid_synth_program_change(m_pSynth, m_iChan, iProg);
        // Maybe this is needed to stabilize things around.
        ::fluid_synth_program_reset(m_pSynth);
        // We got it.
        accept();
    }
}


// Find the bank item of given bank number id.
QListViewItem *qsynthPresetForm::findBankItem ( int iBank )
{
    return BankListView->findItem(QString::number(iBank), 0);
}


// Find the program item of given program number id.
QListViewItem *qsynthPresetForm::findProgItem ( int iProg )
{
    return ProgListView->findItem(QString::number(iProg), 0);
}



// Bank change slot.
void qsynthPresetForm::bankChanged (void)
{
    if (m_pSynth == NULL)
        return;

    QListViewItem *pBankItem = BankListView->selectedItem();
    if (pBankItem == NULL)
        pBankItem = BankListView->currentItem();
    if (pBankItem == NULL)
        return;
    int iBank = pBankItem->text(0).toInt();

    // Clear up the program listview.
    ProgListView->clear();
    // Start freeze...
    BankListView->setUpdatesEnabled(false);
    ProgListView->setUpdatesEnabled(false);
    QListViewItem *pProgItem = NULL;
    fluid_preset_t preset;
    // For all soundfonts (in reversed stack order) fill the available programs...
    int cSoundFonts = ::fluid_synth_sfcount(m_pSynth);
    for (int i = 0; i < cSoundFonts; i++) {
        fluid_sfont_t *pSoundFont = ::fluid_synth_get_sfont(m_pSynth, i);
        if (pSoundFont) {
            pSoundFont->iteration_start(pSoundFont);
            while (pSoundFont->iteration_next(pSoundFont, &preset)) {
                int iProg = preset.get_num(&preset);
                if (iBank == preset.get_banknum(&preset) && !findProgItem(iProg)) {
                    pProgItem = new QListViewItem(ProgListView, pProgItem);
                    if (pProgItem) {
                        pProgItem->setText(0, QString::number(iProg));
                        pProgItem->setText(1, preset.get_name(&preset));
                        pProgItem->setText(2, QString::number(pSoundFont->id));
                        pProgItem->setText(3, QFileInfo(pSoundFont->get_name(pSoundFont)).baseName());
                    }
                }
            }
        }
    }
    // Freeze's over.
    BankListView->setUpdatesEnabled(true);
    ProgListView->setUpdatesEnabled(true);

    // Stabilize the form.
    stabilizeForm();
}


// end of qsynthPresetForm.ui.h
