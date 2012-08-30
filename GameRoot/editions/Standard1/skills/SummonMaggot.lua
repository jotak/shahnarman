childEffectsCount = 1
childEffectsCost = {{0,0,2,0}}	-- 2 death
childEffectsIcon = {"asticot"}
icon = "asticot"

function getName()
	if language == "french" then
		return "Invoquer un asticot des ténèbres"
	else
		return "Summon dark maggot"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Invoquer un asticot des ténèbres"
	else
		return "Summon dark maggot"
	end
end

function getDescription()
	if language == "french" then
		return "Coût d'activation : 2µ. Invoque un terrible asticot des ténèbre."
	else
		return "Activation cost: 2µ. Summon a terrible dark maggot."
	end
end

_player = nil
_unit = nil
function setAttachedUnit(params)
	objtype, _player, _unit = split(params, " ")
end

function onActivateEffect(effect)
end

function onResolveChild(effect, params)
	if effect == 1 then
		x, y = getObjectPosition("unit", _player, _unit)
		summon(_player, x, y, "Dark_Maggot")
	end
end
