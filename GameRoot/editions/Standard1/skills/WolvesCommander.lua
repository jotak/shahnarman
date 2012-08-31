childEffectsCount = 1
childEffectsCost = {{0,0,0,0}}
childEffectsIcon = {"wolvescommander"}
icon = "wolvescommander"

function getName()
	if language == "french" then
		return "Commandant des loups"
	else
		return "Wolves commander"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Commandant des loups"
	else
		return "Wolves commander"
	end
end

function getDescription()
	if language == "french" then
		return "Augmente de 2 la mêlée et l'endurance de tous les chevaucheurs de loup sur la même case."
	else
		return "Increases by 2 melee and endurance of all wolf riders on the same tile."
	end
end

_nbChildren = 0
_children = {}

function onNewUnitTurn(unitids)
	objtype, thisplayer, thisunit = parseint(unitids, " ")
	thisx, thisy = getObjectPosition("unit", thisplayer, thisunit)
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
					break
				end
				iChild = iChild + 1
			end
			unitx, unity = getObjectPosition("unit", player, unit)
			if thisx == unitx and thisy == unity and thisplayer == player then
				if bIsAlreadyAttached == 0 then
					name, edition = getUnitNameAndEdition(player, unit)
					if name == "GalatesWolveRider" then
						--add attachment
						_nbChildren = _nbChildren + 1
						addChildEffectToUnit(1, player, unit)
						_children[_nbChildren] = {player, unit}
					end
				end
			else
				if bIsAlreadyAttached == 1 then
					--remove attachment
					removeChildEffectFromUnit(1, _children[iChild][1], _children[iChild][2])
					table.remove(_children, iChild)
					_nbChildren = _nbChildren - 1
				end
			end
		end
	end
end

function child_getMod_melee(effect, value)
	if effect == 1 then
		return value + 2
	end
	return value
end

function child_getMod_endurance(effect, value)
	if effect == 1 then
		return value + 2
	end
	return value
end
