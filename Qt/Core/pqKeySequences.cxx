// SPDX-FileCopyrightText: Copyright (c) Kitware Inc.
// SPDX-FileCopyrightText: Copyright (c) Sandia Corporation
// SPDX-License-Identifier: BSD-3-Clause
#include "pqKeySequences.h"
#include "pqModalShortcut.h"

#include <QAction>
#include <QKeySequence>
#include <QMap>
#include <QPointer>
#include <QSet>

#include <iostream>

namespace
{

struct Shortcuts
{
  QSet<pqModalShortcut*> Siblings;
  // TODO: Allow a key-sequence to be "default to last" when a shortcut is
  //       removed/disabled as an option?
  // TODO: Hold tooltip or preference name?
};

struct Dictionary
{
  QMap<QKeySequence, Shortcuts> Data;
};

static Dictionary g_keys;

} // anonymous namespace

pqKeySequences& pqKeySequences::instance()
{
  static pqKeySequences inst(nullptr);
  return inst;
}

pqKeySequences::pqKeySequences(QObject* parent)
  : QObject(parent)
  , m_silence(false)
{
}

pqModalShortcut* pqKeySequences::active(const QKeySequence& keySequence) const
{
  pqModalShortcut* active = nullptr;
  QMap<QKeySequence, Shortcuts>::iterator iter = g_keys.Data.find(keySequence);
  if (iter == g_keys.Data.end())
  {
    return active;
  }
  for (auto& sibling : iter->Siblings)
  {
    if (sibling && sibling->isEnabled())
    {
      active = sibling;
      break;
    }
  }
  return active;
}

pqModalShortcut* pqKeySequences::addModalShortcut(
  const QKeySequence& keySequence, QAction* action, QWidget* parent)
{
  if (keySequence.isEmpty())
  {
    return nullptr;
  }
  auto shortcut = new pqModalShortcut(keySequence, action, parent);
  auto& keyData = g_keys.Data[keySequence];
  keyData.Siblings.insert(shortcut);
  shortcut->setEnabled(false);
  QObject::connect(shortcut, &pqModalShortcut::enabled, this, &pqKeySequences::disableSiblings);
  // QObject::connect(shortcut, &pqModalShortcut::disabled, this,
  // &pqKeySequences::enableNextSibling);
  QObject::connect(
    shortcut, &pqModalShortcut::unregister, this, &pqKeySequences::removeModalShortcut);
  shortcut->setEnabled(true); // invoke disableSiblings()
  return shortcut;
}

void pqKeySequences::reorder(pqModalShortcut* target)
{
  (void)target;
}

void pqKeySequences::dumpShortcuts(const QKeySequence& keySequence) const
{
  QMap<QKeySequence, Shortcuts>::iterator iter = g_keys.Data.find(keySequence);
  if (iter == g_keys.Data.end())
  {
    return;
  }
  for (auto& sibling : iter->Siblings)
  {
    std::cout
      << sibling->objectName().toStdString() << "   "
      << (sibling->isEnabled() ? "ena" : "dis")
      // << " → " << (sibling->m_next ? sibling->m_next->objectName().toStdString() : "null")
      << "\n";
  }
}

void pqKeySequences::disableSiblings()
{
  if (m_silence)
  {
    return;
  }
  auto* shortcut = qobject_cast<pqModalShortcut*>(this->sender());
  if (!shortcut)
  {
    return;
  }
  QMap<QKeySequence, Shortcuts>::iterator iter = g_keys.Data.find(shortcut->keySequence());
  if (iter == g_keys.Data.end())
  {
    return;
  }
  m_silence = true;
  for (auto& sibling : iter->Siblings)
  {
    if (sibling != shortcut)
    {
      sibling->setEnabled(false);
    }
  }
  m_silence = false;
}

void pqKeySequences::enableNextSibling()
{
  if (m_silence)
  {
    return;
  }
  auto* shortcut = qobject_cast<pqModalShortcut*>(this->sender());
  if (!shortcut)
  {
    return;
  }
  // TODO: Activate a sibling of shortcut that is the most-recently-used
  //       and also not a child of a disabled widget.
}

void pqKeySequences::removeModalShortcut()
{
  auto* shortcut = qobject_cast<pqModalShortcut*>(this->sender());
  if (!shortcut)
  {
    return;
  }
  QMap<QKeySequence, Shortcuts>::iterator iter = g_keys.Data.find(shortcut->keySequence());
  if (iter == g_keys.Data.end())
  {
    return;
  }

  // Remove the shortcut from the key sequence list and delete it.
  iter->Siblings.remove(shortcut);
  QObject::disconnect(shortcut);
}
