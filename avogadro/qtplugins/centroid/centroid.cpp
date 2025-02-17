/******************************************************************************
  This source file is part of the Avogadro project.
  This source code is released under the 3-Clause BSD License, (see "LICENSE").
******************************************************************************/

#include "centroid.h"

#include <avogadro/core/elements.h>
#include <avogadro/qtgui/molecule.h>

#include <QAction>

namespace Avogadro::QtPlugins {

using Core::Array;
using Core::Elements;

Centroid::Centroid(QObject* parent_)
  : Avogadro::QtGui::ExtensionPlugin(parent_),
    m_centroidAction(new QAction(tr("Add Centroid"), this)),
    m_comAction(new QAction(tr("Add Center of Mass"), this)),
    m_normalAction(new QAction(
      tr("Add Perpendicular", "add a point normal to the plane of the molecule"),
      this))
{
  m_centroidAction->setProperty("menu priority", 190);
  m_comAction->setProperty("menu priority", 180);
  m_normalAction->setProperty("menu priority", 170);

  connect(m_centroidAction, SIGNAL(triggered()), SLOT(addCentroid()));
  connect(m_comAction, SIGNAL(triggered()), SLOT(addCenterOfMass()));
  connect(m_normalAction, SIGNAL(triggered()), SLOT(normal()));
}

Centroid::~Centroid() {}

QList<QAction*> Centroid::actions() const
{
  QList<QAction*> result;
  return result << m_centroidAction << m_comAction << m_normalAction;
}

QStringList Centroid::menuPath(QAction*) const
{
  return QStringList() << tr("&Build");
}

void Centroid::setMolecule(QtGui::Molecule* mol)
{
  m_molecule = mol;
}

void Centroid::addCentroid()
{
  if (m_molecule->isSelectionEmpty()) {
    m_molecule->addAtom(0, m_molecule->centerOfGeometry());
  } else {
    Vector3 center;
    Index selectedCount = 0;
    for (Index i = 0; i < m_molecule->atomCount(); ++i) {
      if (!m_molecule->atomSelected(i))
        continue;

      center += m_molecule->atomPosition3d(i);
      ++selectedCount;
    }
    center /= selectedCount;

    m_molecule->addAtom(0, center);
  }

  m_molecule->emitChanged(QtGui::Molecule::Atoms | QtGui::Molecule::Added);
}

void Centroid::addCenterOfMass()
{
  if (m_molecule->isSelectionEmpty()) {
    m_molecule->addAtom(0, m_molecule->centerOfMass());
  } else {
    Vector3 center;
    Index selectedCount = 0;
    Real totalMass = 0.0;

    for (Index i = 0; i < m_molecule->atomCount(); ++i) {
      if (!m_molecule->atomSelected(i))
        continue;

      Real mass = Elements::mass(m_molecule->atomicNumber(i));
      center += m_molecule->atomPosition3d(i) * mass;

      totalMass += mass;
      ++selectedCount;
    }
    center /= selectedCount;
    center /= totalMass;

    m_molecule->addAtom(0, center);
  }

  m_molecule->emitChanged(QtGui::Molecule::Atoms | QtGui::Molecule::Added);
}

void Centroid::normal()
{
  if (m_molecule->isSelectionEmpty()) {
    auto pair = m_molecule->bestFitPlane();
    m_molecule->addAtom(0.0, pair.second * 2.0);
  } else {
    Array<Vector3> selectedAtoms;
    for (Index i = 0; i < m_molecule->atomCount(); ++i) {
      if (!m_molecule->atomSelected(i))
        continue;

      selectedAtoms.push_back(m_molecule->atomPosition3d(i));
    }

    auto pair = m_molecule->bestFitPlane(selectedAtoms);
    Vector3 newPos = pair.second * 2.0 + pair.first;
    m_molecule->addAtom(0, newPos);
  }

  m_molecule->emitChanged(QtGui::Molecule::Atoms | QtGui::Molecule::Added);
}

} // namespace Avogadro::QtPlugins
