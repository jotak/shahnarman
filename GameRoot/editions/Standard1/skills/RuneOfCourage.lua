childEffectsCount = 1
childEffectsCost = {{0,0,0,2}}
childEffectsIcon = {"runeofcourage"}
icon = "runeofcourage"

function getName()
	if language == "french" then
		return "Rune de courage"
	else
		return "Rune of courage"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Rune de courage"
	else
		return "Rune of courage"
	end
end

function getDescription()
	if language == "french" then
		return "Coût d'activation : 2§. Augmente de 2 la mêlée de tous les alliés sur la même case."
	else
		return "Activation cost: 2§. Increases by 2 melee of all friend units on the same tile."
	end
end

function onActivateEffect(effect)
end

_nbChildren = 0
_children = {}

function onResolveChild(effect, params, caster, unit)
	if effect == 1 then
		-- add child to every friend units on tile
		thisx, thisy = getObjectPosition("unit", caster, unit)

		-- every turn, loop through units to attach child to units on tile
		players = {getPlayersList()}
		for p, player in pairs(players) do
			units = {getUnitsList(player)}
			for u, unit in pairs(units) do
				unitx, unity = getObjectPosition("unit", player, unit)
				if thisx == unitx and thisy == unity and caster == player then
					--add attachment
					_nbChildren = _nbChildren + 1
					addChildEffectToUnit(1, player, unit)
					_children[_nbChildren] = {player, unit}
				end
			end
		end
	end
end

function onNewUnitTurn(unitids)
	-- remove all child effects
	while _nbChildren > 0 do
		--remove attachment
		removeChildEffectFromUnit(1, _children[_nbChildren][1], _children[_nbChildren][2])
		table.remove(_children, _nbChildren)
		_nbChildren = _nbChildren - 1
	end
end

function child_getMod_melee(effect, value)
	if effect == 1 then
		return value + 2
	end
	return value
end
