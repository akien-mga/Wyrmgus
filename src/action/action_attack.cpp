//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name action_attack.cpp - The attack action. */
//
//      (c) Copyright 1998-2015 by Lutz Sammer, Jimmy Salmon and Andrettin
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//

//@{

/**
**  @todo FIXME: I should rewrite this action, if only the
**               new orders are supported.
*/

/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "action/action_attack.h"

#include "animation.h"
//Wyrmgus start
#include "commands.h"
//Wyrmgus end
#include "iolib.h"
#include "map.h"
#include "missile.h"
#include "pathfinder.h"
#include "player.h"
#include "script.h"
#include "settings.h"
#include "sound.h"
#include "spells.h"
#include "tileset.h"
#include "ui.h"
#include "unit.h"
#include "unit_find.h"
#include "unittype.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Defines
----------------------------------------------------------------------------*/

#define WEAK_TARGET      2  /// Weak target, could be changed
#define MOVE_TO_TARGET   4  /// Move to target state
#define ATTACK_TARGET    5  /// Attack target state

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Animate unit attack!
**
**  @param unit  Unit, for that the attack animation is played.
**
**  @todo manage correctly unit with no animation attack.
*/
void AnimateActionAttack(CUnit &unit, COrder &order)
{
	//  No animation.
	//  So direct fire missile.
	//  FIXME : wait a little.
	//Wyrmgus start
	/*
	if (unit.Type->Animations && unit.Type->Animations->RangedAttack && unit.IsAttackRanged(order.GetGoal(), order.GetGoalPos())) {
		UnitShowAnimation(unit, unit.Type->Animations->RangedAttack);
	} else {
		if (!unit.Type->Animations || !unit.Type->Animations->Attack) {
			order.OnAnimationAttack(unit);
			return;
		}
		UnitShowAnimation(unit, unit.Type->Animations->Attack);
	}
	*/
	if (unit.GetAnimations() && unit.GetAnimations()->RangedAttack && unit.IsAttackRanged(order.GetGoal(), order.GetGoalPos(), order.GetGoalMapLayer())) {
		UnitShowAnimation(unit, unit.GetAnimations()->RangedAttack);
	} else {
		if (!unit.GetAnimations() || !unit.GetAnimations()->Attack) {
			order.OnAnimationAttack(unit);
			return;
		}
		UnitShowAnimation(unit, unit.GetAnimations()->Attack);
	}
	//Wyrmgus end
}

/* static */ COrder *COrder::NewActionAttack(const CUnit &attacker, CUnit &target)
{
	COrder_Attack *order = new COrder_Attack(false);

	order->goalPos = target.tilePos + target.Type->GetHalfTileSize();
	//Wyrmgus start
	order->MapLayer = target.MapLayer;
	//Wyrmgus end
	// Removed, Dying handled by action routine.
	order->SetGoal(&target);
	//Wyrmgus start
//	order->Range = attacker.Stats->Variables[ATTACKRANGE_INDEX].Max;
	order->Range = attacker.GetModifiedVariable(ATTACKRANGE_INDEX);
	//Wyrmgus end
	order->MinRange = attacker.Type->MinAttackRange;

	//Wyrmgus start
	if (!attacker.Type->BoolFlag[HIDDENOWNERSHIP_INDEX].value && !target.Type->BoolFlag[HIDDENOWNERSHIP_INDEX].value && !target.IsEnemy(attacker) && (target.Player->Type == PlayerComputer) && (attacker.Player->Type == PlayerComputer || attacker.Player->Type == PlayerPerson)) {
		target.Player->SetDiplomacyEnemyWith(*attacker.Player);
		attacker.Player->SetDiplomacyEnemyWith(*target.Player);
		if (target.Player->IsSharedVision(*attacker.Player)) {
			CommandSharedVision(target.Player->Index, false, attacker.Player->Index);
		}
	}
	//Wyrmgus end

	return order;
}

//Wyrmgus start
///* static */ COrder *COrder::NewActionAttack(const CUnit &attacker, const Vec2i &dest)
/* static */ COrder *COrder::NewActionAttack(const CUnit &attacker, const Vec2i &dest, int z)
//Wyrmgus end
{
	//Wyrmgus start
//	Assert(Map.Info.IsPointOnMap(dest));
	Assert(Map.Info.IsPointOnMap(dest, z));
	//Wyrmgus end

	COrder_Attack *order = new COrder_Attack(false);

	//Wyrmgus start
//	if (Map.WallOnMap(dest) && Map.Field(dest)->playerInfo.IsExplored(*attacker.Player)) {
	if (Map.WallOnMap(dest, z) && Map.Field(dest, z)->playerInfo.IsTeamExplored(*attacker.Player)) {
	//Wyrmgus end
		// FIXME: look into action_attack.cpp about this ugly problem
		order->goalPos = dest;
		//Wyrmgus start
		order->MapLayer = z;
//		order->Range = attacker.Stats->Variables[ATTACKRANGE_INDEX].Max;
		order->Range = attacker.GetModifiedVariable(ATTACKRANGE_INDEX);
		//Wyrmgus end
		order->MinRange = attacker.Type->MinAttackRange;
	} else {
		order->goalPos = dest;
		//Wyrmgus start
		order->MapLayer = z;
		//Wyrmgus end
	}
	return order;
}

//Wyrmgus start
///* static */ COrder *COrder::NewActionAttackGround(const CUnit &attacker, const Vec2i &dest)
/* static */ COrder *COrder::NewActionAttackGround(const CUnit &attacker, const Vec2i &dest, int z)
//Wyrmgus end
{
	COrder_Attack *order = new COrder_Attack(true);

	order->goalPos = dest;
	//Wyrmgus start
	order->MapLayer = z;
//	order->Range = attacker.Stats->Variables[ATTACKRANGE_INDEX].Max;
	order->Range = attacker.GetModifiedVariable(ATTACKRANGE_INDEX);
	//Wyrmgus end
	order->MinRange = attacker.Type->MinAttackRange;

	return order;
}


/* virtual */ void COrder_Attack::Save(CFile &file, const CUnit &unit) const
{
	Assert(Action == UnitActionAttack || Action == UnitActionAttackGround);

	if (Action == UnitActionAttack) {
		file.printf("{\"action-attack\",");
	} else {
		file.printf("{\"action-attack-ground\",");
	}
	file.printf(" \"range\", %d,", this->Range);
	file.printf(" \"min-range\", %d,", this->MinRange);

	if (this->Finished) {
		file.printf(" \"finished\", ");
	}
	if (this->HasGoal()) {
		file.printf(" \"goal\", \"%s\",", UnitReference(this->GetGoal()).c_str());
	}
	file.printf(" \"tile\", {%d, %d},", this->goalPos.x, this->goalPos.y);
	//Wyrmgus start
	file.printf(" \"map-layer\", %d,", this->MapLayer);
	//Wyrmgus end

	file.printf(" \"state\", %d", this->State);
	file.printf("}");
}


/* virtual */ bool COrder_Attack::ParseSpecificData(lua_State *l, int &j, const char *value, const CUnit &unit)
{
	if (!strcmp(value, "state")) {
		++j;
		this->State = LuaToNumber(l, -1, j + 1);
	} else if (!strcmp(value, "min-range")) {
		++j;
		this->MinRange = LuaToNumber(l, -1, j + 1);
	} else if (!strcmp(value, "range")) {
		++j;
		this->Range = LuaToNumber(l, -1, j + 1);
	} else if (!strcmp(value, "tile")) {
		++j;
		lua_rawgeti(l, -1, j + 1);
		CclGetPos(l, &this->goalPos.x , &this->goalPos.y);
		lua_pop(l, 1);
	//Wyrmgus start
	} else if (!strcmp(value, "map-layer")) {
		++j;
		this->MapLayer = LuaToNumber(l, -1, j + 1);
	//Wyrmgus end
	} else {
		return false;
	}
	return true;
}

/* virtual */ bool COrder_Attack::IsValid() const
{
	if (Action == UnitActionAttack) {
		if (this->HasGoal()) {
			return this->GetGoal()->IsAliveOnMap();
		} else {
			//Wyrmgus start
//			return Map.Info.IsPointOnMap(this->goalPos);
			return Map.Info.IsPointOnMap(this->goalPos, this->MapLayer);
			//Wyrmgus end
		}
	} else {
		Assert(Action == UnitActionAttackGround);
		//Wyrmgus start
//		return Map.Info.IsPointOnMap(this->goalPos);
		return Map.Info.IsPointOnMap(this->goalPos, this->MapLayer);
		//Wyrmgus end
	}
}

/* virtual */ PixelPos COrder_Attack::Show(const CViewport &vp, const PixelPos &lastScreenPos) const
{
	PixelPos targetPos;

	if (this->HasGoal()) {
		//Wyrmgus start
		if (this->GetGoal()->MapLayer != CurrentMapLayer) {
			return lastScreenPos;
		}
		//Wyrmgus end
		targetPos = vp.MapToScreenPixelPos(this->GetGoal()->GetMapPixelPosCenter());
	} else {
		//Wyrmgus start
		if (this->MapLayer != CurrentMapLayer) {
			return lastScreenPos;
		}
		//Wyrmgus end
		targetPos = vp.TilePosToScreen_Center(this->goalPos);
	}
	//Wyrmgus start
//	Video.FillCircleClip(ColorRed, lastScreenPos, 2);
//	Video.DrawLineClip(ColorRed, lastScreenPos, targetPos);
//	Video.FillCircleClip(IsWeakTargetSelected() ? ColorBlue : ColorRed, targetPos, 3);
	if (Preference.ShowPathlines) {
		Video.FillCircleClip(ColorRed, lastScreenPos, 2);
		Video.DrawLineClip(ColorRed, lastScreenPos, targetPos);
		Video.FillCircleClip(IsWeakTargetSelected() ? ColorBlue : ColorRed, targetPos, 3);
	}
	//Wyrmgus end
	return targetPos;
}

/* virtual */ void COrder_Attack::UpdatePathFinderData(PathFinderInput &input)
{
	Vec2i tileSize;
	if (this->HasGoal()) {
		CUnit *goal = this->GetGoal();
		tileSize.x = goal->Type->TileWidth;
		tileSize.y = goal->Type->TileHeight;
		//Wyrmgus start
//		input.SetGoal(goal->tilePos, tileSize);
		input.SetGoal(goal->tilePos, tileSize, goal->MapLayer);
		//Wyrmgus end
	} else {
		tileSize.x = 0;
		tileSize.y = 0;
		//Wyrmgus start
//		input.SetGoal(this->goalPos, tileSize);
		input.SetGoal(this->goalPos, tileSize, this->MapLayer);
		//Wyrmgus end
	}

	input.SetMinRange(this->MinRange);
	int distance = this->Range;
	//Wyrmgus start
//	if (GameSettings.Inside) {
//		CheckObstaclesBetweenTiles(input.GetUnitPos(), this->HasGoal() ? this->GetGoal()->tilePos : this->goalPos, MapFieldRocks | MapFieldForest, &distance);
//	}
	if (Map.IsLayerUnderground(this->MapLayer) && input.GetUnit()->GetModifiedVariable(ATTACKRANGE_INDEX) > 1) {
		if (!CheckObstaclesBetweenTiles(input.GetUnitPos(), this->HasGoal() ? this->GetGoal()->tilePos : this->goalPos, MapFieldAirUnpassable, this->MapLayer)) {
			distance = 1;
		}
	}
	//Wyrmgus end
	input.SetMaxRange(distance);
}

/* virtual */ void COrder_Attack::OnAnimationAttack(CUnit &unit)
{
	//Wyrmgus start
//	Assert(unit.Type->CanAttack);
	if (!unit.CanAttack()) {
		return;
	}
	//Wyrmgus end

	//Wyrmgus start
//	FireMissile(unit, this->GetGoal(), this->goalPos);
	FireMissile(unit, this->GetGoal(), this->goalPos, this->MapLayer);
	//Wyrmgus end
	UnHideUnit(unit); // unit is invisible until attacks
}

/* virtual */ bool COrder_Attack::OnAiHitUnit(CUnit &unit, CUnit *attacker, int /*damage*/)
{
	CUnit *goal = this->GetGoal();

	if (goal) {
		if (goal->IsAlive() == false) {
			this->ClearGoal();
			this->goalPos = goal->tilePos;
			//Wyrmgus start
			this->MapLayer = goal->MapLayer;
			//Wyrmgus end
			return false;
		}
		if (goal == attacker) {
			return true;
		}
		//Wyrmgus start
//		if (goal->CurrentAction() == UnitActionAttack) {
		if (goal->CurrentAction() == UnitActionAttack && unit.MapDistanceTo(*goal) <= unit.GetModifiedVariable(ATTACKRANGE_INDEX)) {
		//Wyrmgus end
			const COrder_Attack &order = *static_cast<COrder_Attack *>(goal->CurrentOrder());
			if (order.GetGoal() == &unit) {
				//we already fight with one of attackers;
				return true;
			}
		}
	}
	return false;
}



bool COrder_Attack::IsWeakTargetSelected() const
{
	return (this->State & WEAK_TARGET) != 0;
}

/**
**  Check for dead goal.
**
**  @warning  The caller must check, if he likes the restored SavedOrder!
**
**  @todo     If a unit enters an building, than the attack choose an
**            other goal, perhaps it is better to wait for the goal?
**
**  @param unit  Unit using the goal.
**
**  @return      true if order have changed, false else.
*/
bool COrder_Attack::CheckForDeadGoal(CUnit &unit)
{
	CUnit *goal = this->GetGoal();

	// Position or valid target, it is ok.
	if (!goal || goal->IsVisibleAsGoal(*unit.Player)) {
		return false;
	}

	// Goal could be destroyed or unseen
	// So, cannot use type.
	this->goalPos = goal->tilePos;
	//Wyrmgus start
	this->MapLayer = goal->MapLayer;
	//Wyrmgus end
	this->MinRange = 0;
	this->Range = 0;
	this->ClearGoal();

	// If we have a saved order continue this saved order.
	if (unit.RestoreOrder()) {
		return true;
	}
	return false;
}

/**
**  Change invalid target for new target in range.
**
**  @param unit  Unit to check if goal is in range
**
**  @return      true if order(action) have changed, false else (if goal change return false).
*/
bool COrder_Attack::CheckForTargetInRange(CUnit &unit)
{
	// Target is dead?
	if (CheckForDeadGoal(unit)) {
		return true;
	}
	
	// No goal: if meeting enemy attack it.
	if (!this->HasGoal()
		&& this->Action != UnitActionAttackGround
		//Wyrmgus start
//		&& !Map.WallOnMap(this->goalPos)) {
		&& !Map.WallOnMap(this->goalPos, this->MapLayer)) {
		//Wyrmgus end
		CUnit *goal = AttackUnitsInReactRange(unit);

		if (goal) {
			//Wyrmgus start
//			COrder *savedOrder = COrder::NewActionAttack(unit, this->goalPos);
			COrder *savedOrder = COrder::NewActionAttack(unit, this->goalPos, this->MapLayer);
			//Wyrmgus end

			if (unit.CanStoreOrder(savedOrder) == false) {
				delete savedOrder;
				savedOrder = NULL;
			} else {
				unit.SavedOrder = savedOrder;
			}
			this->SetGoal(goal);
			this->MinRange = unit.Type->MinAttackRange;
			//Wyrmgus start
//			this->Range = unit.Stats->Variables[ATTACKRANGE_INDEX].Max;
			this->Range = unit.GetModifiedVariable(ATTACKRANGE_INDEX);
			//Wyrmgus end
			this->goalPos = goal->tilePos;
			//Wyrmgus start
			this->MapLayer = goal->MapLayer;
			//Wyrmgus end
			this->State |= WEAK_TARGET; // weak target
		}
		// Have a weak target, try a better target.
	} else if (this->HasGoal() && (this->State & WEAK_TARGET || unit.Player->AiEnabled)) {
		CUnit *goal = this->GetGoal();
		CUnit *newTarget = AttackUnitsInReactRange(unit);

		if (newTarget && ThreatCalculate(unit, *newTarget) < ThreatCalculate(unit, *goal)) {
			COrder *savedOrder = NULL;
			if (unit.CanStoreOrder(this)) {
				savedOrder = this->Clone();
			}
			if (savedOrder != NULL) {
				unit.SavedOrder = savedOrder;
			}
			this->SetGoal(newTarget);
			this->goalPos = newTarget->tilePos;
			//Wyrmgus start
			this->MapLayer = newTarget->MapLayer;
			//Wyrmgus end
		}
	}

	Assert(!unit.Type->BoolFlag[VANISHES_INDEX].value && !unit.Destroyed && !unit.Removed);
	return false;
}

/**
**  Controls moving a unit to its target when attacking
**
**  @param unit  Unit that is attacking and moving
*/
void COrder_Attack::MoveToTarget(CUnit &unit)
{
	Assert(!unit.Type->BoolFlag[VANISHES_INDEX].value && !unit.Destroyed && !unit.Removed);
	Assert(unit.CurrentOrder() == this);
	Assert(unit.CanMove());
	//Wyrmgus start
//	Assert(this->HasGoal() || Map.Info.IsPointOnMap(this->goalPos));
	Assert(this->HasGoal() || Map.Info.IsPointOnMap(this->goalPos, this->MapLayer));
	//Wyrmgus end

	//Wyrmgus start
	//if is on a moving raft and target is now within range, stop the raft
	if ((Map.Field(unit.tilePos, unit.MapLayer)->Flags & MapFieldBridge) && !unit.Type->BoolFlag[BRIDGE_INDEX].value && unit.Type->UnitType == UnitTypeLand) {
		std::vector<CUnit *> table;
		Select(unit.tilePos, unit.tilePos, table, unit.MapLayer);
		for (size_t i = 0; i != table.size(); ++i) {
			if (!table[i]->Removed && table[i]->Type->BoolFlag[BRIDGE_INDEX].value && table[i]->CanMove()) {
				if (table[i]->CurrentAction() == UnitActionMove) {
					if ((this->GetGoal() && unit.MapDistanceTo(*this->GetGoal()) <= unit.GetModifiedVariable(ATTACKRANGE_INDEX)) || (!this->HasGoal() && unit.MapDistanceTo(this->goalPos, this->MapLayer) <= unit.GetModifiedVariable(ATTACKRANGE_INDEX))) {
						if (!Map.IsLayerUnderground(this->MapLayer) || CheckObstaclesBetweenTiles(unit.tilePos, goalPos, MapFieldAirUnpassable, MapLayer)) {
							CommandStopUnit(*table[i]);
						}
					}
				}
			}
		}
	}
	//Wyrmgus end
				
	int err = DoActionMove(unit);

	if (unit.Anim.Unbreakable) {
		return;
	}

	// Look if we have reached the target.
	if (err == 0 && !this->HasGoal()) {
		// Check if we're in range when attacking a location and we are waiting
		//Wyrmgus start
//		if (unit.MapDistanceTo(this->goalPos) <= unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
		if (unit.MapDistanceTo(this->goalPos, this->MapLayer) <= unit.GetModifiedVariable(ATTACKRANGE_INDEX)) {
		//Wyrmgus end
			//Wyrmgus start
//			if (!GameSettings.Inside || CheckObstaclesBetweenTiles(unit.tilePos, goalPos, MapFieldRocks | MapFieldForest)) {
			if (!Map.IsLayerUnderground(MapLayer) || CheckObstaclesBetweenTiles(unit.tilePos, goalPos, MapFieldAirUnpassable, MapLayer)) {
			//Wyrmgus end
				err = PF_REACHED;
			}
		}
	}
	if (err >= 0) {
		if (CheckForTargetInRange(unit)) {
			return;
		}
		return;
	}
	if (err == PF_REACHED) {
		CUnit *goal = this->GetGoal();
		// Have reached target? FIXME: could use the new return code?
		//Wyrmgus start
//		if (goal && unit.MapDistanceTo(*goal) <= unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
		if (goal && unit.MapDistanceTo(*goal) <= unit.GetModifiedVariable(ATTACKRANGE_INDEX)) {
		//Wyrmgus end
			//Wyrmgus start
//			if (!GameSettings.Inside || CheckObstaclesBetweenTiles(unit.tilePos, goalPos, MapFieldRocks | MapFieldForest)) {
			if (!Map.IsLayerUnderground(this->MapLayer) || CheckObstaclesBetweenTiles(unit.tilePos, goalPos, MapFieldAirUnpassable, MapLayer)) {
			//Wyrmgus end
				// Reached another unit, now attacking it
				unsigned char oldDir = unit.Direction;
				//Wyrmgus start
//				const Vec2i dir = goal->tilePos + goal->Type->GetHalfTileSize() - unit.tilePos;
				const Vec2i dir = Vec2i(goal->tilePos.x * PixelTileSize.x, goal->tilePos.y * PixelTileSize.y) + goal->Type->GetHalfTilePixelSize() - Vec2i(unit.tilePos.x * PixelTileSize.x, unit.tilePos.y * PixelTileSize.y) - unit.Type->GetHalfTilePixelSize();
				//Wyrmgus end
				UnitHeadingFromDeltaXY(unit, dir);
				if (unit.Type->BoolFlag[SIDEATTACK_INDEX].value) {
					unsigned char leftTurn = (unit.Direction - 2 * NextDirection) % (NextDirection * 8);
					unsigned char rightTurn = (unit.Direction + 2 * NextDirection) % (NextDirection * 8);
					if (abs(leftTurn - oldDir) < abs(rightTurn - oldDir)) {
						unit.Direction = leftTurn;
					} else {
						unit.Direction = rightTurn;
					}
					UnitUpdateHeading(unit);
				}
				this->State++;
				return;
			}
		}
		// Attacking wall or ground.
		if (((goal && goal->Type && goal->Type->BoolFlag[WALL_INDEX].value)
			//Wyrmgus start
//			 || (!goal && (Map.WallOnMap(this->goalPos) || this->Action == UnitActionAttackGround)))
			 || (!goal && (this->Action == UnitActionAttackGround || Map.WallOnMap(this->goalPos, this->MapLayer))))
			//Wyrmgus end
			//Wyrmgus start
//			&& unit.MapDistanceTo(this->goalPos) <= unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
			&& unit.MapDistanceTo(this->goalPos, this->MapLayer) <= unit.GetModifiedVariable(ATTACKRANGE_INDEX)) {
			//Wyrmgus end
			//Wyrmgus start
//			if (!GameSettings.Inside || CheckObstaclesBetweenTiles(unit.tilePos, goalPos, MapFieldRocks | MapFieldForest)) {
			if (!Map.IsLayerUnderground(this->MapLayer) || CheckObstaclesBetweenTiles(unit.tilePos, goalPos, MapFieldAirUnpassable, MapLayer)) {
			//Wyrmgus end
				// Reached wall or ground, now attacking it
				unsigned char oldDir = unit.Direction;
				UnitHeadingFromDeltaXY(unit, this->goalPos - unit.tilePos);
				if (unit.Type->BoolFlag[SIDEATTACK_INDEX].value) {
					unsigned char leftTurn = (unit.Direction - 2 * NextDirection) % (NextDirection * 8);
					unsigned char rightTurn = (unit.Direction + 2 * NextDirection) % (NextDirection * 8);
					if (abs(leftTurn - oldDir) < abs(rightTurn - oldDir)) {
						unit.Direction = leftTurn;
					} else {
						unit.Direction = rightTurn;
					}
					UnitUpdateHeading(unit);
				}
				this->State &= WEAK_TARGET;
				this->State |= ATTACK_TARGET;
				return;
			}
		}
	}
	// Unreachable.

	if (err == PF_UNREACHABLE) {
		//Wyrmgus start
		//if is unreachable and is on a raft, see if the raft can move closer to the enemy
		if ((Map.Field(unit.tilePos, unit.MapLayer)->Flags & MapFieldBridge) && !unit.Type->BoolFlag[BRIDGE_INDEX].value && unit.Type->UnitType == UnitTypeLand) {
			std::vector<CUnit *> table;
			Select(unit.tilePos, unit.tilePos, table, unit.MapLayer);
			for (size_t i = 0; i != table.size(); ++i) {
				if (!table[i]->Removed && table[i]->Type->BoolFlag[BRIDGE_INDEX].value && table[i]->CanMove()) {
					if (table[i]->CurrentAction() == UnitActionStill) {
						CommandStopUnit(*table[i]);
						CommandMove(*table[i], this->HasGoal() ? this->GetGoal()->tilePos : this->goalPos, FlushCommands, this->HasGoal() ? this->GetGoal()->MapLayer : this->MapLayer);
					}
					return;
				}
			}
		}
		//Wyrmgus end
		if (!this->HasGoal()) {
			// When attack-moving we have to allow a bigger range
			this->Range++;
			unit.Wait = 5;
			return;
		} else {
			this->ClearGoal();
		}
	}

	// Return to old task?
	if (!unit.RestoreOrder()) {
		this->Finished = true;
	}
}

/**
**  Handle attacking the target.
**
**  @param unit  Unit, for that the attack is handled.
*/
void COrder_Attack::AttackTarget(CUnit &unit)
{
	//Wyrmgus start
//	Assert(this->HasGoal() || Map.Info.IsPointOnMap(this->goalPos));
	Assert(this->HasGoal() || Map.Info.IsPointOnMap(this->goalPos, this->MapLayer));
	//Wyrmgus end

	AnimateActionAttack(unit, *this);
	if (unit.Anim.Unbreakable) {
		return;
	}

	//Wyrmgus start
//	if (!this->HasGoal() && (this->Action == UnitActionAttackGround || Map.WallOnMap(this->goalPos))) {
	if (!this->HasGoal() && (this->Action == UnitActionAttackGround || Map.WallOnMap(this->goalPos, this->MapLayer))) {
	//Wyrmgus end
		return;
	}

	// Target is dead ? Change order ?
	if (CheckForDeadGoal(unit)) {
		return;
	}
	CUnit *goal = this->GetGoal();
	bool dead = !goal || goal->IsAlive() == false;

	// No target choose one.
	if (!goal) {
		goal = AttackUnitsInReactRange(unit);

		// No new goal, continue way to destination.
		if (!goal) {
			// Return to old task ?
			if (unit.RestoreOrder()) {
				return;
			}
			this->State = MOVE_TO_TARGET;
			return;
		}
		// Save current command to come back.
		//Wyrmgus start
//		COrder *savedOrder = COrder::NewActionAttack(unit, this->goalPos);
		COrder *savedOrder = COrder::NewActionAttack(unit, this->goalPos, this->MapLayer);
		//Wyrmgus end

		if (unit.CanStoreOrder(savedOrder) == false) {
			delete savedOrder;
			savedOrder = NULL;
		} else {
			unit.SavedOrder = savedOrder;
		}
		this->SetGoal(goal);
		this->goalPos = goal->tilePos;
		//Wyrmgus start
		this->MapLayer = goal->MapLayer;
		//Wyrmgus end
		this->MinRange = unit.Type->MinAttackRange;
		//Wyrmgus start
//		this->Range = unit.Stats->Variables[ATTACKRANGE_INDEX].Max;
		this->Range = unit.GetModifiedVariable(ATTACKRANGE_INDEX);
		//Wyrmgus end
		this->State |= WEAK_TARGET;

		// Have a weak target, try a better target.
		// FIXME: if out of range also try another target quick
	} else {
		if ((this->State & WEAK_TARGET)) {
			CUnit *newTarget = AttackUnitsInReactRange(unit);
			if (newTarget && ThreatCalculate(unit, *newTarget) < ThreatCalculate(unit, *goal)) {
				if (unit.CanStoreOrder(this)) {
					unit.SavedOrder = this->Clone();
				}
				goal = newTarget;
				this->SetGoal(newTarget);
				this->goalPos = newTarget->tilePos;
				//Wyrmgus start
				this->MapLayer = newTarget->MapLayer;
				//Wyrmgus end
				this->MinRange = unit.Type->MinAttackRange;
				this->State = MOVE_TO_TARGET;
			}
		}
	}

	// Still near to target, if not goto target.
	const int dist = unit.MapDistanceTo(*goal);
	//Wyrmgus start
//	if (dist > unit.Stats->Variables[ATTACKRANGE_INDEX].Max
	if (dist > unit.GetModifiedVariable(ATTACKRANGE_INDEX)
	//Wyrmgus end
		//Wyrmgus start
//		|| (GameSettings.Inside && CheckObstaclesBetweenTiles(unit.tilePos, goal->tilePos, MapFieldRocks | MapFieldForest) == false)) {
		|| (Map.IsLayerUnderground(this->MapLayer) && CheckObstaclesBetweenTiles(unit.tilePos, goal->tilePos, MapFieldAirUnpassable, MapLayer) == false)) {
		//Wyrmgus end
		//Wyrmgus start
		// towers don't chase after goal
		/*
		if (unit.CanMove()) {
			if (unit.CanStoreOrder(this)) {
				if (dead) {
					//Wyrmgus start
//					unit.SavedOrder = COrder::NewActionAttack(unit, this->goalPos);
					unit.SavedOrder = COrder::NewActionAttack(unit, this->goalPos, this->MapLayer);
					//Wyrmgus end
				} else {
					unit.SavedOrder = this->Clone();
				}
			}
		}
		*/
		//Wyrmgus end
		unit.Frame = 0;
		this->State &= WEAK_TARGET;
		this->State |= MOVE_TO_TARGET;
	}
	if (dist < unit.Type->MinAttackRange) {
		this->State = MOVE_TO_TARGET;
	}

	// Turn always to target
	if (goal) {
		//Wyrmgus start
//		const Vec2i dir = goal->tilePos + goal->Type->GetHalfTileSize() - unit.tilePos;
		const Vec2i dir = Vec2i(goal->tilePos.x * PixelTileSize.x, goal->tilePos.y * PixelTileSize.y) + goal->Type->GetHalfTilePixelSize() - Vec2i(unit.tilePos.x * PixelTileSize.x, unit.tilePos.y * PixelTileSize.y) - unit.Type->GetHalfTilePixelSize();
		//Wyrmgus end
		unsigned char oldDir = unit.Direction;
		UnitHeadingFromDeltaXY(unit, dir);
		if (unit.Type->BoolFlag[SIDEATTACK_INDEX].value) {
			unsigned char leftTurn = (unit.Direction - 2 * NextDirection) % (NextDirection * 8);
			unsigned char rightTurn = (unit.Direction + 2 * NextDirection) % (NextDirection * 8);
			if (abs(leftTurn - oldDir) < abs(rightTurn - oldDir)) {
				unit.Direction = leftTurn;
			} else {
				unit.Direction = rightTurn;
			}
			UnitUpdateHeading(unit);
		}
	}
}

/**
**  Unit attacks!
**
**  if (SubAction & WEAK_TARGET) is true the goal is a weak goal.
**  This means the unit AI (little AI) could choose a new better goal.
**
**  @todo  Lets do some tries to reach the target.
**         If target place is not reachable, choose better goal to reduce
**         the pathfinder load.
**
**  @param unit  Unit, for that the attack is handled.
*/
/* virtual */ void COrder_Attack::Execute(CUnit &unit)
{
	//Wyrmgus start
//	Assert(this->HasGoal() || Map.Info.IsPointOnMap(this->goalPos));
	Assert(this->HasGoal() || Map.Info.IsPointOnMap(this->goalPos, this->MapLayer));
	//Wyrmgus end

	if (unit.Wait) {
		if (!unit.Waiting) {
			unit.Waiting = 1;
			unit.WaitBackup = unit.Anim;
		}
		//Wyrmgus start
//		UnitShowAnimation(unit, unit.Type->Animations->Still);
		UnitShowAnimation(unit, unit.GetAnimations()->Still);
		//Wyrmgus end
		unit.Wait--;
		return;
	}
	if (unit.Waiting) {
		unit.Anim = unit.WaitBackup;
		unit.Waiting = 0;
	}
	
	//Wyrmgus start
	if (!unit.CanAttack(true) && !this->HasGoal()) { //if unit is a transporter that can't attack, return false if the original target no longer exists
		this->Finished = true;
		return;
	}
	//Wyrmgus end

	switch (this->State) {
		case 0: { // First entry
			// did Order change ?
			if (CheckForTargetInRange(unit)) {
				return;
			}
			// Can we already attack ?
			if (this->HasGoal()) {
				CUnit &goal = *this->GetGoal();
				const int dist = goal.MapDistanceTo(unit);

				if (unit.Type->MinAttackRange < dist &&
					//Wyrmgus start
//					dist <= unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
					dist <= unit.GetModifiedVariable(ATTACKRANGE_INDEX)) {
					//Wyrmgus end
					//Wyrmgus start
//					if (!GameSettings.Inside || CheckObstaclesBetweenTiles(unit.tilePos, goalPos, MapFieldRocks | MapFieldForest)) {
					if (!Map.IsLayerUnderground(this->MapLayer) || CheckObstaclesBetweenTiles(unit.tilePos, goalPos, MapFieldAirUnpassable, MapLayer)) {
					//Wyrmgus end
						//Wyrmgus start
//						const Vec2i dir = goal.tilePos + goal.Type->GetHalfTileSize() - unit.tilePos;
						const Vec2i dir = Vec2i(goal.tilePos.x * PixelTileSize.x, goal.tilePos.y * PixelTileSize.y) + goal.Type->GetHalfTilePixelSize() - Vec2i(unit.tilePos.x * PixelTileSize.x, unit.tilePos.y * PixelTileSize.y) - unit.Type->GetHalfTilePixelSize();
						//Wyrmgus end
						unsigned char oldDir = unit.Direction;
						UnitHeadingFromDeltaXY(unit, dir);
						if (unit.Type->BoolFlag[SIDEATTACK_INDEX].value) {
							unsigned char leftTurn = (unit.Direction - 2 * NextDirection) % (NextDirection * 8);
							unsigned char rightTurn = (unit.Direction + 2 * NextDirection) % (NextDirection * 8);
							if (abs(leftTurn - oldDir) < abs(rightTurn - oldDir)) {
								unit.Direction = leftTurn;
							} else {
								unit.Direction = rightTurn;
							}
							UnitUpdateHeading(unit);
						}
						this->State |= ATTACK_TARGET;
						AttackTarget(unit);
						return;
					}
				}
			//Wyrmgus start
			// add instance for attack ground without moving
			//Wyrmgus start
//			} else if (this->Action == UnitActionAttackGround && unit.MapDistanceTo(this->goalPos) <= unit.Stats->Variables[ATTACKRANGE_INDEX].Max) {
			} else if (this->Action == UnitActionAttackGround && unit.MapDistanceTo(this->goalPos, this->MapLayer) <= unit.GetModifiedVariable(ATTACKRANGE_INDEX)) {
			//Wyrmgus end
				if (!Map.IsLayerUnderground(this->MapLayer) || CheckObstaclesBetweenTiles(unit.tilePos, goalPos, MapFieldAirUnpassable, MapLayer)) {
					// Reached wall or ground, now attacking it
					unsigned char oldDir = unit.Direction;
					UnitHeadingFromDeltaXY(unit, this->goalPos - unit.tilePos);
					if (unit.Type->BoolFlag[SIDEATTACK_INDEX].value) {
						unsigned char leftTurn = (unit.Direction - 2 * NextDirection) % (NextDirection * 8);
						unsigned char rightTurn = (unit.Direction + 2 * NextDirection) % (NextDirection * 8);
						if (abs(leftTurn - oldDir) < abs(rightTurn - oldDir)) {
							unit.Direction = leftTurn;
						} else {
							unit.Direction = rightTurn;
						}
						UnitUpdateHeading(unit);
					}
					this->State &= WEAK_TARGET;
					this->State |= ATTACK_TARGET;
					return;
				}
			//Wyrmgus end
			}
			this->State = MOVE_TO_TARGET;
			// FIXME: should use a reachable place to reduce pathfinder time.
		}
		// FALL THROUGH
		case MOVE_TO_TARGET:
		case MOVE_TO_TARGET + WEAK_TARGET:
			if (!unit.CanMove()) {
				this->Finished = true;
				return;
			}
			MoveToTarget(unit);
			break;

		case ATTACK_TARGET:
		case ATTACK_TARGET + WEAK_TARGET:
			AttackTarget(unit);
			break;

		case WEAK_TARGET:
			DebugPrint("FIXME: wrong entry.\n");
			break;
	}
}

//@}
