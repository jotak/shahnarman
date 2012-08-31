childEffectsCount = 1
childEffectsCost = {{0,2,0,0}}	-- 2 law
childEffectsIcon = {"charming"}
icon = "charming"

function getName()
	if language == "french" then
		return "Charme"
	else
		return "Charming"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Charme"
	else
		return "Charming"
	end
end

function getDescription()
	if language == "french" then
		return "Coût d'activation : 2|. Prend le contrôle de l'unité ciblée (hors Shahmah) pendant un tour. Cette unité perd 1 point de mêlée, de tir et de vitesse."
	else
		return "Activation cost: 2|. Take control of target unit (except Shahmah) for 1 turn. This unit looses 1 point of melee, range and speed."
	end
end

function onActivateEffect(effect)
	if effect == 1 then
		selectTarget("unit", "opponent not_shahmah")
	end
end

_charmedUnit = -1
_charmedPlayer = -1
_turnCounter = 0

function releaseCurrentUnit(thisplayer)
	if _charmedUnit >= 0 and _charmedPlayer >= 0 then
		changeUnitOwner(thisplayer, _charmedUnit, _charmedPlayer)
		removeChildEffectFromUnit(1, _charmedUnit, _charmedPlayer)
		_charmedUnit = -1
		_charmedPlayer = -1
		_turnCounter = 0
	end
end

-- this function receives playedId and unitId as parameter, in a space-separated string
function onResolveChild(effect, params, caster)
	if effect == 1 then
		-- First, release current charmed unit
		releaseCurrentUnit(caster)
		player, unit = splitint(params, " ")
		changeUnitOwner(player, unit, caster)
		addChildEffectToUnit(1, caster, unit)
		_turnCounter = 2
	end
end

function onNewUnitTurn(unitids)
	if _turnCounter == 1 then
		objtype, player, unit = splitint(unitids)
		releaseCurrentUnit(player)
	end
end

function child_getMod_melee(effect, value)
	if effect == 1 and value > 0 then
		return value - 1
	end
	return value
end

function child_getMod_range(effect, value)
	if effect == 1 and value > 0 then
		return value - 1
	end
	return value
end

function child_getMod_speed(effect, value)
	if effect == 1 and value > 0 then
		return value - 1
	end
	return value
end
