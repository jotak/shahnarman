cost = {5,0,0,0}
icon = "killingcreeper"
allowedInBattle = 0

childEffectsCount = 1
childEffectsCost = {{0,0,0,0}}
childEffectsIcon = {"killingcreeper"}

function getName()
	if language == "french" then
		return "Lianes tueuses"
	else
		return "Killing creepers"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Lianes tueuses"
	else
		return "Killing creepers"
	end
end

function getDescription()
	if language == "french" then
		return "Les lianes tueuses doivent être lancées sur une case de forêt. Les unités présentes sur cette case ne peuvent plus se déplacer, et reçoivent 1 point de dégât chaque tour."
	else
		return "The killing creepers must be cast on a forest. Units on that tile cannot move, and receive 1 damage every turn."
	end
end

function onCast()
	selectTarget("tile", "inrange custom")
end

function onCheckSelect(tile)
	x, y = splitint(tile, " ")
	terrain = getTileData(x, y, "terrain")
	if terrain == "forest" then
		return 1
	end
	return 0
end

_x = 0
_y = 0
_nbChildren = 0
_children = {}

function onResolve(params)
	_x, _y = splitint(params, " ")
	attachToTile(_x, _y)
end

function onNewTileTurn()
	-- every turn, loop through units to attach child to units on tile (if not already attached)
	players = {getPlayersList()}
	for p, player in pairs(players) do
		units = {getUnitsList(player)}
		for u, unit in pairs(units) do
			bIsAlreadyAttached = 0
			iChild = 1
			while iChild <= _nbChildren do
				if player == _children[iChild][1] and unit == _children[iChild][2] then
					bIsAlreadyAttached = 1
					break;
				end
				iChild = iChild + 1
			end
			unitx, unity = getObjectPosition("unit", player, unit)
			if _x == unitx and _y == unity then
				damageUnit(player, unit, 1)
				if bIsAlreadyAttached == 0 then
					--add attachment
					_nbChildren = _nbChildren + 1
					addChildEffectToUnit(1, player, unit)
					_children[_nbChildren] = {player, unit}
				end
			end
			if (_x ~= unitx or _y ~= unity) and bIsAlreadyAttached == 1 then
				--remove attachment
				removeChildEffectFromUnit(1, _children[iChild][1], _children[iChild][2])
				table.remove(_children, iChild)
				_nbChildren = _nbChildren - 1
			end
		end
	end
end

function child_getMod_speed(effect, value)
	if effect == 1 then
		return 0
	end
	return value
end
