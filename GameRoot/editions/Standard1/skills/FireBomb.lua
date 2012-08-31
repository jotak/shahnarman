childEffectsCount = 1
childEffectsCost = {{0,0,0,0}}
childEffectsIcon = {"firebomb"}
icon = "firebomb"

_power = 1
_reloadTurns = 1
_turnsCounter = 0

function getName()
	if language == "french" then
		return "Bombe incendiaire"
	else
		return "Fire bomb"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Bombe incendiaire"
	else
		return "Fire bomb"
	end
end

function getDescription()
	if language == "french" then
		return "Inflige ".._power.." dégâts à la cible. Ne peut être utilisé qu'une fois tous les ".._reloadTurns.." tours."
	else
		return "Deals ".._power.." damages to target unit. Can only be used once every ".._reloadTurns.." turns."
	end
end

function init(params)
	if params ~= nil and params ~= "" then
		_power, _reloadTurns = splitint(params, " ")
	end
end

function onActivateEffect(effect)
	if effect == 1 then
		selectTarget("unit", "inrange")
	end
end

function onResolveChild(effect, params)
	if effect == 1 then
		player, unit = splitint(params, " ");
		damageUnit(player, unit, _power)
		_turnsCounter = _reloadTurns
	end
end

function onNewUnitTurn(unitids)
	if _turnsCounter > 0 then
		deactivateSkill()
		_turnsCounter = _turnsCounter - 1
	end
end

isMergeable = 1
function merge(params)
	reloadTurnsMerge = 1
	powerMerge = 1
	if params ~= nil and params ~= "" then
		powerMerge, reloadTurnsMerge = splitint(params, " ")
	end
	if reloadTurnsMerge < _reloadTurns then
		_reloadTurns = reloadTurnsMerge
	end
	if powerMerge > _power then
		_power = powerMerge
	end
	return "".._power.." ".._reloadTurns
end
