childEffectsCount = 1
childEffectsCost = {{0,0,0,0}}
childEffectsIcon = {"agitator"}
icon = "agitator"

function getName()
	if language == "french" then
		return "Agitateur"
	else
		return "Agitator"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Agitateur"
	else
		return "Agitator"
	end
end

function getDescription()
	if language == "french" then
		return "Diminue de 2 la peur et le bonheur dans toutes les villes ennemies."
	else
		return "Decreases by 2 fear and happiness in every ennemy town."
	end
end

_nbChildren = 0
_children = {}

function onNewUnitTurn(unitids)
	objtype, thisplayer, thisunit = parseint(unitids, " ")
	thisx, thisy = getObjectPosition("unit", thisplayer, thisunit)
	-- every turn, loop through towns to attach child to units on tile (if not already attached)
	towns = {getPlayersList()}
	for t, town in pairs(towns) do
		bIsAlreadyAttached = 0
		iChild = 1
		while iChild <= _nbChildren do
			if town == _children[iChild] then
				bIsAlreadyAttached = 1
				break
			end
			iChild = iChild + 1
		end
		name, ethn, player = getTownData(town)
		if thisplayer ~= player then
			if bIsAlreadyAttached == 0 then
				--add attachment
				_nbChildren = _nbChildren + 1
				addChildEffectToTown(1, town)
				_children[_nbChildren] = town
			end
		else
			if bIsAlreadyAttached == 1 then
				--remove attachment
				removeChildEffectFromTown(1, _children[iChild])
				table.remove(_children, iChild)
				_nbChildren = _nbChildren - 1
			end
		end
	end
end

function child_getMod_fear(effect, value)
	if effect == 1 then
		return value - 4
	end
	return value
end

function child_getMod_happiness(effect, value)
	if effect == 1 then
		return value - 4
	end
	return value
end
