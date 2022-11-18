/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2019 CERN
 * Copyright (C) 2019-2021 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef KICAD_SCH_MOVE_TOOL_H
#define KICAD_SCH_MOVE_TOOL_H

#include <tools/ee_tool_base.h>
#include <sch_base_frame.h>


class SCH_EDIT_FRAME;
class EE_SELECTION_TOOL;


struct SPECIAL_CASE_LABEL_INFO
{
    SCH_LINE* attachedLine;
    VECTOR2I  originalLabelPos;
};


class SCH_MOVE_TOOL : public EE_TOOL_BASE<SCH_EDIT_FRAME>
{
public:
    SCH_MOVE_TOOL();
    ~SCH_MOVE_TOOL() override { }

    /// @copydoc TOOL_INTERACTIVE::Init()
    bool Init() override;

    /**
     * Run an interactive move of the selected items, or the item under the cursor.
     */
    int Main( const TOOL_EVENT& aEvent );

    /**
     * Align selected elements to the grid.
     *
     * @param aEvent current event that activated the tool
     * @return 0
     */
    int AlignElements( const TOOL_EVENT& aEvent );

private:
    void moveItem( EDA_ITEM* aItem, const VECTOR2I& aDelta );

    ///< Find additional items for a drag operation.
    ///< Connected items with no wire are included (as there is no wire to adjust for the drag).
    ///< Connected wires are included with any un-connected ends flagged (STARTPOINT or ENDPOINT).
    void getConnectedItems( SCH_ITEM* aOriginalItem, const VECTOR2I& aPoint, EDA_ITEMS& aList );
    void getConnectedDragItems( SCH_ITEM* fixed, const VECTOR2I& selected, EDA_ITEMS& aList );

    void orthoLineDrag( SCH_LINE* line, const VECTOR2I& splitDelta, int& xBendCount,
                        int& yBendCount, const EE_GRID_HELPER& grid );

    ///< Saves the new drag lines to the undo list
    void commitDragLines();

    ///< Clears the new drag lines and removes them from the screen
    void clearNewDragLines();

    ///< Set up handlers for various events.
    void setTransitions() override;

private:
    ///< Re-entrancy guard
    bool                  m_inMoveTool;

    ///< Flag determining if anything is being dragged right now
    bool                  m_moveInProgress;
    bool                  m_isDrag;

    ///< Items (such as wires) which were added to the selection for a drag
    std::vector<KIID>                   m_dragAdditions;
    ///< Cache of the line's original connections before dragging started
    std::map<SCH_LINE*, EDA_ITEMS>      m_lineConnectionCache;
    ///< Lines added at bend points dynamically during the move
    std::unordered_set<SCH_LINE*>       m_newDragLines;
    ///< Lines changed by drag algorithm that weren't selected
    std::unordered_set<SCH_LINE*>       m_changedDragLines;

    ///< Used for chaining commands
    VECTOR2I              m_moveOffset;

    ///< Last cursor position (needed for getModificationPoint() to avoid changes
    ///< of edit reference point).
    VECTOR2I              m_cursor;

    OPT_VECTOR2I          m_anchorPos;

    // A map of labels to scaling factors.  Used to scale the movement vector for labels that
    // are attached to wires which have only one end moving.
    std::map<SCH_LABEL_BASE*, SPECIAL_CASE_LABEL_INFO> m_specialCaseLabels;

    // A map of sheet pins to the line-endings (true == start) they're connected to.  Sheet
    // pins are constrained in their movement so their attached lines must be too.
    std::map<SCH_SHEET_PIN*, std::pair<SCH_LINE*, bool>> m_specialCaseSheetPins;
};

#endif //KICAD_SCH_MOVE_TOOL_H
