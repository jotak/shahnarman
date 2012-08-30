childEffectsCount = 1
childEffectsCost = {{0,0,0,0}}
childEffectsIcon = {"ane"}
icon = "ane"

function getName()
	if language == "french" then
		return "Soigneur"
	else
		return "Healer"
	end
end

function getChildEffectsName()
	if language == "french" then
		return "Soins"
	else
		return "Heal"
	end
end

function getDescription()
	if language == "french" then
		return "Soigne la cible de 2 points de dégâts."
	else
		return "Heals 2 hits points on target unit."
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
		life, endurance = getUnitData(player, unit, "life", "endurance")
		if life + 2 > endurance then
			setUnitData(player, unit, "life", endurance)
		else
			setUnitData(player, unit, "life", life+2)
		end
	end
end
