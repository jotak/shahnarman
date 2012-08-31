childEffectsCount = 1
childEffectsCost = {{0,0,0,0}}
childEffectsIcon = {"recruitment"}
icon = "recruitment"

function getName()
	if language == "french" then
		return "Recrutement"
	else
		return "Recruitment"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Recrutement"
	else
		return "Recruitment"
	end
end

function getDescription()
	if language == "french" then
		return "Lorsque l'unité se trouve sur une ville amie, la vitesse de formation d'unités dans cette ville est doublée."
	else
		return "When the unit is in a friend town, the units recruitment speed is doubled in this town."
	end
end

_town = -1

function onNewUnitTurn(unitids)
	objtype, thisplayer, thisunit = split(unitids, " ")
	thisx, thisy = getObjectPosition("unit", thisplayer, thisunit)
	-- every turn, loop through towns to attach child to town on tile (if not already attached)
	towns = {getTownsList()}
	for t, town in pairs(towns) do
		townx, towny = getObjectPosition("town", town)
		name, ethn, player = getTownData(town)
		if thisx == townx and thisy == towny and player == 0+thisplayer then
			if _town ~= town then
				if _town >= 0 then
					-- remove previous attachment
					removeChildEffectFromTown(1, _town)
					_town = -1
				end
				--add attachment
				addChildEffectToTown(1, town)
				_town = town
			end
		else
			if _town == town then
				--remove attachment
				removeChildEffectFromTown(1, _town)
				_town = -1
			end
		end
	end
end

function child_getMod_unitprod(effect, value)
	if effect == 1 then
		return value / 2	-- unitprod: lower is better (unitprod is the percentage of basis unit cost)
	end
	return value
end
