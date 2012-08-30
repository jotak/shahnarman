childEffectsCount = 1
childEffectsCost = {{1,0,0,0}}	-- 1 life
childEffectsIcon = {"ane"}
icon = "ane"

function getName()
	if language == "french" then
		return "R�g�n�ration"
	else
		return "Regenerate"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "R�g�n�rer"
	else
		return "Regenerate"
	end
end

function getDescription()
	if language == "french" then
		return "Co�t d'activation : 1�. L'unit� soigne toutes ses blessures."
	else
		return "Activation cost: 1�. The unit heals all its damages."
	end
end

_player = nil
_unit = nil
function setAttachedUnit(params)
	objtype, _player, _unit = split(params, " ")
end

function onActivateEffect(effect)
--	if effect == 1 then
--	end
end

function onResolveChild(effect)
	if effect == 1 then
		endurance = getUnitData(_player, _unit, "endurance")
		setUnitData(_player, _unit, "life", endurance)
	end
end
