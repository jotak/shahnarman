childEffectsCount = 1
childEffectsCost = {{0,0,0,0}}
childEffectsIcon = {"ane"}
icon = "ane"

function getName()
	if language == "french" then
		return "Eclair des fées"
	else
		return "Sprites bolt"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Eclair des fées"
	else
		return "Sprites bolt"
	end
end

function getDescription()
	if language == "french" then
		return "Inflige 2 dégâts à la cible."
	else
		return "Deals 2 damages to target unit."
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
		damageUnit(player, unit, 2)
	end
end
