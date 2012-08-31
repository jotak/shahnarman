childEffectsCount = 1
childEffectsCost = {{1,0,0,0}}	-- 1 life
childEffectsIcon = {"holyblast"}
icon = "holyblast"

function getName()
	if language == "french" then
		return "Coup sacr�"
	else
		return "Holy blast"
	end
end

function getChildEffectsName(language)
	if language == "french" then
		return "Coup sacr�"
	else
		return "Holy blast"
	end
end

function getDescription()
	if language == "french" then
		return "Co�t d'activation : 1�. Inflige 2 d�g�ts � la cible. S'il s'agit d'une unit� d'alignement MORT, inflige 1 d�g�t suppl�mentaire."
	else
		return "Activation cost: 1�. Deals 2 damages on target unit. If its alignment is DEATH, deals 1 extra damage."
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
		life, law, death, chaos = getUnitAlignment(player, unit)
		if death == 1 then
			damageUnit(player, unit, 3)
		else
			damageUnit(player, unit, 2)
		end
	end
end
