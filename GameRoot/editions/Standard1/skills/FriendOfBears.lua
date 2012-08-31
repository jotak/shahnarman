childEffectsCount = 1
childEffectsCost = {{0,0,0,0}}
childEffectsIcon = {"warbears"}

icon = "warbears"
_reloadTurns = 1
_turnsCounter = 0

function getName()
	if language == "french" then
		return "Ami des ours"
	else
		return "Friend of bears"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Ami des ours"
	else
		return "Friend of bears"
	end
end

function getDescription()
	if language == "french" then
		return "Peut invoquer des ours de guerre tous les ".._reloadTurns.." tours."
	else
		return "Can summon war bears every ".._reloadTurns.." turns."
	end
end

function init(params)
	if params ~= nil and params ~= "" then
		_reloadTurns = params+0
	end
end

function onActivateEffect(effect)
	if effect == 1 then
		selectTarget("tile", "inrange")
	end
end

function onResolveChild(effect, params, caster)
	if effect == 1 then
		x, y = splitint(params, " ");
		summon(caster, x, y, "WarBears")
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
	if params ~= nil and params ~= "" then
		reloadTurnsMerge = params+0
	end
	if reloadTurnsMerge < _reloadTurns then
		_reloadTurns = reloadTurnsMerge
	end
	return "".._reloadTurns
end
