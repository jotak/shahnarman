cost = {0,0,6,0}
icon = "plague"
allowedInBattle = 0

childEffectsCount = 1
childEffectsCost = {{0,0,0,0}}
childEffectsIcon = {"plague"}

function getName()
	if language == "french" then
		return "Peste"
	else
		return "Plague"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Peste"
	else
		return "Plague"
	end
end

function getDescription()
	if language == "french" then
		return "La peste cible initialement une unité, puis se répand à toutes les unités sur la même case ou sur une case adjacente. Les unités touchées par la peste perdent 2 points de vie par tour et leurs caractéristiques de mêlée et de tir sont divisées par 2."
	else
		return "The plague affects one unit initially, then propagates to other units on the same tile and on adjacent tiles. Affected units take 2 damages each turn and see their melee and range characteristics divided by 2."
	end
end

function onCast()
	selectTarget("unit", "")
end

_nbUnits = 0
_units = {}

function onResolve(params)
	attachAsGlobal()
	player, unit = splitint(params, " ")
	addChildEffectToUnit(1, player, unit)
	_nbUnits = 1
	_units[1] = { player, unit }
end

function onNewTurn()
	--first check if there's removed stored units
	iUnit = 1
	while iUnit <= _nbUnits do
		state = getUnitStatus(_units[iUnit][1], _units[iUnit][2])
		if state == "removed" then
			table.remove(_units, iUnit)
			_nbUnits = _nbUnits - 1
		else
			iUnit = iUnit + 1
		end
	end
	if _nbUnits == 0 then
		discardActiveSpell()
		return
	end
	-- every turn, loop through units to attach effects to units on near tiles (if not already attached)
	players = {getPlayersList()}
	for p, player in pairs(players) do
		units = {getUnitsList(player)}
		for u, unit in pairs(units) do
			unitx, unity = getObjectPosition("unit", player, unit)
			bIsAlreadyAttached = 0
			bIsPlaguable = 0
			iUnit = 1
			-- loop through current affected units
			while iUnit <= _nbUnits do
				-- if already in list, then ignore it
				if player == _units[iUnit][1] and unit == _units[iUnit][2] then
					bIsAlreadyAttached = 1
					break
				end
				-- see if it's "plaguable" (i.e. near another affected unit, even dead unit (plague also propagates from dead corpses), but not "removed" units)
				if bIsPlaguable == 0 then
					unitx2, unity2 = getObjectPosition("unit", _units[iUnit][1], _units[iUnit][2])
					if math.abs(unitx-unitx2) <= 1 and math.abs(unity-unity2) <= 1 then
						bIsPlaguable = 1
					end
				end
				iUnit = iUnit + 1
			end
			if bIsPlaguable == 1 and bIsAlreadyAttached == 0 then
				addChildEffectToUnit(1, player, unit)
				_nbUnits = _nbUnits + 1
				_units[_nbUnits] = { player, unit }
			end
		end
	end
end

function child_onNewUnitTurn(effect, unitids)
	if effect == 1 then
		objtype, player, unit = splitint(unitids, " ")
		damageUnit(player, unit, 2)
	end
end

function child_getMod_melee(effect, value)
	if effect == 1 then
		return value / 2
	end
	return value
end

function child_getMod_range(effect, value)
	if effect == 1 then
		return value / 2
	end
	return value
end
